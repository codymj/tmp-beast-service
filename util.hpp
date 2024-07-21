#pragma once

#include <boost/beast/core.hpp>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

inline void fail(beast::error_code const& ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << '\n';
}

inline void log(std::string&& msg)
{
    std::cout << msg << '\n';
}
