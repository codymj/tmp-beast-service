#pragma once

#include "handler.hpp"
#include <memory>
#include <utility>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class status_handler final
: public handler
{
public:
    explicit status_handler(std::unique_ptr<handler> next = nullptr)
    : m_next_handler(std::move(next))
    {}

    http::message_generator handle
    (
        http::request<http::string_body> req,
        http::response<http::string_body> res
    ) override
    {
        res.result(http::status::not_found);
        res.set(http::field::server, "beast");
        res.set(http::field::content_type, "application/json");
        res.keep_alive(req.keep_alive());
        res.body() = R"({"status":"ok"})";

        if (m_next_handler)
        {
            return m_next_handler->handle
            (
                std::move(req),
                std::move(res)
            );
        }

        res.prepare_payload();
        return res;
    }

private:
    std::unique_ptr<handler> m_next_handler;
};
