#pragma once

#include <boost/beast/core.hpp>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include "util.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class handler
: public std::enable_shared_from_this<handler>
{
public:
    explicit handler
    (
        tcp::socket& socket,
        http::request<http::string_body>&& req,
        http::response<http::string_body>&& res
    )
    : m_socket(socket)
    , m_req(std::move(req))
    , m_res(std::move(res))
    {}

    void handle()
    {
        log("Preparing response.");
        m_res.result(http::status::ok);
        m_res.set(http::field::server, "Beast");
        m_res.set(http::field::content_type, "application/json");
        m_res.keep_alive(m_req.keep_alive());
        m_res.body() = R"({"status":"ok"})";
        m_res.prepare_payload();

        auto const me = shared_from_this();
        http::async_write
        (
            m_socket,
            m_res,
            [me](beast::error_code ec, std::size_t bytes_sent)
            {
                boost::ignore_unused(bytes_sent);

                if (!ec && me->m_res.need_eof())
                {
                    log("Socket closed.");
                    ec = me->m_socket.shutdown
                    (
                        tcp::socket::shutdown_send,
                        ec
                    );
                }

                if (ec)
                {
                    fail(ec, "write");
                }

                log("Response sent.\n");
            }
        );
    }

private:
    tcp::socket& m_socket;
    http::request<http::string_body> m_req;
    http::response<http::string_body> m_res;
};
