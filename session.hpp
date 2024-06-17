#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include "handler.hpp"
#include "util.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class session : public std::enable_shared_from_this<session>
{
public:
    explicit session(tcp::socket socket)
    : m_socket(std::move(socket))
    , m_strand(net::make_strand(socket.get_executor()))
    {}

    void run()
    {
        do_read();
    }

private:
    void do_read()
    {
        auto self = shared_from_this();
        async_read
        (
            m_socket,
            m_buffer,
            m_req,
            [self](beast::error_code const ec, std::size_t const bytes_sent)
            {
                boost::ignore_unused(bytes_sent);

                if (ec)
                {
                    fail(ec, "read");
                    return;
                }

                std::cout << "Sending request to handler." << '\n';
                self->to_handler();
            }
        );
    }

    void to_handler()
    {
        std::make_shared<handler>
        (
            m_socket,
            std::move(m_req),
            std::move(http::response<http::string_body>{})
        )->handle();
    }

    tcp::socket m_socket;
    beast::flat_buffer m_buffer;
    http::request<http::string_body> m_req;
    net::strand<net::io_context::executor_type> m_strand;
};