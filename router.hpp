// #pragma once
//
// #include <boost/beast/core.hpp>
// #include <functional>
// #include <map>
// #include <string>
// #include "handler.hpp"
//
// namespace beast = boost::beast;
// namespace http = beast::http;
// namespace net = boost::asio;
// using tcp = net::ip::tcp;
//
// using http_method = std::string const;
// using route_path = std::string const;
// using route_key = std::pair<http_method, route_path>;
// using handler_func = std::function<std::shared_ptr<handler>()>;
//
// namespace codymj
// {
//
// class router
// {
// public:
//     router()
//     {
//         create_routes();
//     }
//
//     std::shared_ptr<handler> lookup_handler(route_key const& key)
//     {
//         handler_func const f = m_routes[key];
//         if (!f)
//         {
//             log("Handler not found.");
//             return std::make_shared<not_found_handler>();
//         }
//         return f();
//     }
//
// private:
//     void create_routes()
//     {
//         create_status_routes();
//     }
//
//     void create_status_routes()
//     {
//         m_routes.insert
//         ({
//             route_key{"GET", "/status"},
//             [&]() -> std::shared_ptr<handler>
//             {
//                 return std::make_shared<status_handler>();
//             }
//         });
//     }
//
//
//     std::map<route_key,handler_func> m_routes{};
// };
//
// }
