#include <boost/beast/core.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/thread_pool.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include "listener.hpp"
#include "util.hpp"

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
    auto const port = static_cast<uint16_t>(std::atoi(argv[2]));
    auto const threads = std::max<int>(1, std::atoi(argv[3]));

    try
    {
        // The io_context is required for all I/O.
        net::thread_pool ioc(threads);

        // Create and launch a listening port.
        std::make_shared<listener>
        (
            ioc.get_executor(),
            tcp::endpoint{address, port}
        )->run();

        // Capture SIGINT and SIGTERM to perform a clean shutdown.
        net::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait
        (
            [&ioc](beast::error_code const&, int)
            {
                log("Stopping I/O context.\n");
                ioc.stop();
            }
        );

        ioc.join();
    }
    catch (beast::system_error const& se)
    {
        fail(se.code(), "listener");
    }

    return EXIT_SUCCESS;
}
