#pragma once

#include "handler.hpp"
#include "util.hpp"
#include <memory>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class not_found_handler final
: public handler
, public std::enable_shared_from_this<not_found_handler>
{
public:
    explicit not_found_handler(handler* next = nullptr)
    : m_next_handler(next)
    {}

    void handle
    (
        tcp::socket& socket,
        http::request<http::string_body> const req,
        http::response<http::string_body> res
    ) override
    {
        log("Preparing 404 response.");
        res.result(http::status::not_found);
        res.set(http::field::server, "beast");
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = R"({"status":"not found"})";
        res.prepare_payload();

        //auto const me = shared_from_this();
        http::async_write
        (
            socket,
            res,
            [&socket, &res](beast::error_code ec, std::size_t bytes_sent)
            {
                if (ec)
                {
                    return fail(ec, "write");
                }

                boost::ignore_unused(bytes_sent);

                if (res.need_eof())
                {
                    log("Socket closed.");
                    ec = socket.shutdown
                    (
                        tcp::socket::shutdown_send,
                        ec
                    );
                }

                log("Response sent.\n");

                // if (me->m_next_handler)
                // {
                //     log("Calling next handler");
                //     m_next_handler->handle
                //     (
                //         socket,
                //         std::move(req),
                //         std::move(res)
                //     );
                // }
            }
        );
    }

private:
    handler* m_next_handler = nullptr;
};
