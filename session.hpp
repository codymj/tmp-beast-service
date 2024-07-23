#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include "router.hpp"
#include "util.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class session
: public std::enable_shared_from_this<session>
{
public:
    explicit session(tcp::socket socket, router* router)
    : m_stream(std::move(socket))
    , m_router(*router)
    {}

    void run()
    {
        do_read();
    }

private:
    void do_read()
    {
        http::async_read
        (
            m_stream,
            m_buffer,
            m_req,
            beast::bind_front_handler
            (
                &session::on_read,
                shared_from_this()
            )
        );
    }

    void on_read(beast::error_code const& ec, std::size_t const bytes_sent)
    {
        boost::ignore_unused(bytes_sent);

        if (ec)
        {
            return fail(ec, "on_read");
        }

        route_key const rk{m_req.method(), m_req.target()};
        auto const handler = m_router.lookup_handler(rk);

        log("Routing request.");
        send_response
        (
            handler->handle(std::move(m_req), std::move(m_res))
        );
    }

    void send_response(http::message_generator&& msg)
    {
        bool keep_alive = msg.keep_alive();

        beast::async_write
        (
            m_stream,
            std::move(msg),
            beast::bind_front_handler
            (
                &session::on_write,
                shared_from_this(),
                keep_alive
            )
        );
    }

    void on_write
    (
        bool const keep_alive,
        beast::error_code const& ec,
        std::size_t bytes_sent
    )
    {
        boost::ignore_unused(bytes_sent);

        if (ec)
        {
            return fail(ec, "on_write");
        }

        if (!keep_alive)
        {
            return do_close();
        }

        log("Sent response.");
        log("");

        do_read();
    }

    void do_close()
    {
        beast::error_code ec;

        ec = m_stream.socket().shutdown(tcp::socket::shutdown_send, ec);

        if (ec)
        {
            return fail(ec, "do_close");
        }
    }

    beast::tcp_stream m_stream;
    beast::flat_buffer m_buffer;
    http::request<http::string_body> m_req{};
    http::response<http::string_body> m_res{};
    router& m_router;
};
