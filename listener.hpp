#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/strand.hpp>
#include <algorithm>
#include <memory>
#include "session.hpp"
#include "util.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class listener
: public std::enable_shared_from_this<listener>
{
public:
    listener(net::io_context& ioc, tcp::endpoint const& endpoint)
    : m_ioc(ioc)
    , m_acceptor(make_strand(ioc))
    , m_strand(make_strand(ioc))
    {
        beast::error_code ec;

        // Open acceptor.
        std::cout << "Opening acceptor." << '\n';
        ec = m_acceptor.open(endpoint.protocol(), ec);
        if (ec)
        {
            fail(ec, "open");
            return;
        }

        // Allow address reuse.
        ec = m_acceptor.set_option(net::socket_base::reuse_address(true), ec);
        if (ec)
        {
            fail(ec, "set_option");
            return;
        }

        // Bind to the server address.
        std::cout << "Binding acceptor." << '\n';
        ec = m_acceptor.bind(endpoint, ec);
        if (ec)
        {
            fail(ec, "bind");
            return;
        }

        // Begin listening for connections.
        std::cout << "Listening on acceptor." << '\n';
        ec = m_acceptor.listen(net::socket_base::max_listen_connections, ec);
        if (ec)
        {
            fail(ec, "listen");
            return;
        }
    }

    // Begin accepting connections.
    void run()
    {
        std::cout << "Accepting connections." << '\n';
        do_accept();
    }

private:
    void do_accept()
    {
        std::cout << "Waiting for a new connection." << '\n';
        auto self = shared_from_this();
        m_acceptor.async_accept
        (
            make_strand(m_ioc),
            beast::bind_front_handler
            (
                &listener::on_accept,
                shared_from_this()
            )
        );
    }

    void on_accept(beast::error_code ec, tcp::socket socket)
    {
        if (!ec)
        {
            std::cout << "Accepted new connection." << '\n';
            std::make_shared<session>(std::move(socket))->run();
        }
        else
        {
            fail(ec, "accept");
        }

        do_accept();
    }

    net::io_context& m_ioc;
    tcp::acceptor m_acceptor;
    net::strand<net::io_context::executor_type> m_strand;
};
