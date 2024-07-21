#pragma once

#include <boost/beast/core.hpp>
#include <functional>
#include <map>
#include <string>
#include "not_found_handler.hpp"
#include "status_handler.hpp"
#include "log_middleware.hpp"
#include "util.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

using route_key = std::pair<http::verb, std::string>;
using handler_func = std::function<std::shared_ptr<handler>()>;

class router
{
public:
    router(router const&) = delete;
    router& operator=(router const&) = delete;
    router(router&&) noexcept = delete;
    router& operator=(router&&) noexcept = delete;

    static router& instance()
    {
        static router r;
        return r;
    }

    std::shared_ptr<handler> lookup_handler(route_key const& key)
    {
        handler_func const f = m_routes[key];
        if (!f)
        {
            log("Handler not found.");
            return std::make_shared<not_found_handler>();
        }

        return f();
    }

private:
    router()
    {
        create_routes();
    }

    void create_routes()
    {
        create_status_routes();
    }

    void create_status_routes()
    {
        m_routes.insert
        ({
            route_key{http::verb::get, "/status"},
            []() -> std::shared_ptr<handler>
            {
                auto const sh = std::make_shared<status_handler>();
                return std::make_shared<log_middleware>(sh);
            }
        });
    }

    std::map<route_key, handler_func> m_routes{};
};
