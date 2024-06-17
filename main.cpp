#include <boost/beast/core.hpp>
#include <boost/asio/signal_set.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "listener.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

int main(int const argc, char* argv[])
{
    if (argc != 4)
    {
        std::cerr << "Usage: beast-service <address> <port> <threads>\n";
        return EXIT_FAILURE;
    }
    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const threads = std::atoi(argv[3]);

    // The io_context is required for all I/O.
    net::io_context ioc;

    // Create and launch a listening port.
    std::make_shared<listener>(ioc, tcp::endpoint{address, port})->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown.
    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait
    (
        [&](beast::error_code const&, int)
        {
            std::cout << "Stopping I/O context." << '\n';
            ioc.stop();
        }
    );

    // Run the I/O service on the requested number of threads.
    std::vector<std::thread> thread_pool;
    thread_pool.reserve(threads);
    for (auto i=0; i<threads; ++i)
    {
        thread_pool.emplace_back
        (
            [i, &ioc]
            {
                std::cout << "Starting I/O context on thread " << i << '\n';
                ioc.run();
            }
        );
    }

    // Block until all the threads exit.
    for (auto i=0; i<threads; ++i)
    {
        std::cout << "Joining thread " << i << '\n';
        thread_pool[i].join();
    }

    return EXIT_SUCCESS;
}
