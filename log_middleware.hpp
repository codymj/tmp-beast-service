#pragma once

#include "handler.hpp"
#include <memory>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class log_middleware final
: public handler
, public std::enable_shared_from_this<log_middleware>
{
public:
    explicit log_middleware(std::shared_ptr<handler> next = nullptr)
    : m_next_handler(std::move(next))
    {}

    http::message_generator handle
    (
        beast::tcp_stream& stream,
        http::request<http::string_body> req,
        http::response<http::string_body> res
    ) override
    {
        std::cout << req.method_string() << " " << req.target() << '\n';

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
