#pragma once

#include "handler.hpp"
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
    explicit not_found_handler(std::shared_ptr<handler> next = nullptr)
    : m_next_handler(std::move(next))
    {}

    http::message_generator handle
    (
        beast::tcp_stream& stream,
        http::request<http::string_body> req,
        http::response<http::string_body> res
    ) override
    {
        res.result(http::status::not_found);
        res.set(http::field::server, "beast");
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = R"({"status":"not found"})";
        res.prepare_payload();

        if (m_next_handler)
        {
            return m_next_handler->handle
            (
                stream,
                std::move(req),
                std::move(res)
            );
        }
        else
        {
            return res;
        }
    }

private:
    std::shared_ptr<handler> m_next_handler = nullptr;
};
