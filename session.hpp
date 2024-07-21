#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include "handler.hpp"
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
    explicit session(tcp::socket socket)
    : m_stream(std::move(socket))
    {}

    void run()
    {
        do_read();
    }

private:
    template <class Body, class Allocator>
    http::message_generator handle_request
    (
        http::request<Body, http::basic_fields<Allocator>>&& req
    )
    {
        auto const bad_request = [&req](beast::string_view const why)
        {
            http::response<http::string_body> res
            {
                http::status::bad_request,
                req.version()
            };
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = R"({"status":"bad request"})";
            res.prepare_payload();
            return res;
        };

        auto const not_found = [&req](beast::string_view const target)
        {
            http::response<http::string_body> res
            {
                http::status::not_found,
                req.version()
            };
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = R"({"status":"not found"})";
            res.prepare_payload();
            return res;
        };

        auto const server_error = [&req](beast::string_view const what)
        {
            http::response<http::string_body> res
            {
                http::status::internal_server_error,
                req.version()
            };
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(req.keep_alive());
            res.body() = R"({"status":"internal error"})";
            res.prepare_payload();
            return res;
        };

        if (req.target() == "/status")
        {
            http::response<http::string_body> res
            {
                http::status::ok,
                req.version()
            };
            res.set(http::field::server, "beast");
            res.set(http::field::content_type, "application/json");
            res.keep_alive(req.keep_alive());
            res.body() = R"({"status":"ok"})";
            res.prepare_payload();
            return res;
        }

        return not_found(m_req.target());
    }

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

        if (ec) return fail(ec, "read");

        send_response(handle_request(std::move(m_req)));
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

        if (ec) return fail(ec, "write");

        if (!keep_alive) return do_close();

        do_read();
    }

    void do_close()
    {
        beast::error_code ec;

        ec = m_stream.socket().shutdown(tcp::socket::shutdown_send, ec);

        if (ec) return fail(ec, "close");
    }

    beast::tcp_stream m_stream;
    beast::flat_buffer m_buffer;
    http::request<http::string_body> m_req{};
};
