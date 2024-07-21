#pragma once

#include <boost/beast/core.hpp>

namespace beast = boost::beast;
namespace http = beast::http;

class handler
{
public:
    virtual ~handler() = default;

    virtual http::message_generator handle
    (
        beast::tcp_stream& stream,
        http::request<http::string_body> req,
        http::response<http::string_body> res
    ) = 0;
};
