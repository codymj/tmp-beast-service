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
    listener(net::any_io_executor executor, tcp::endpoint const& endpoint)
    : m_executor(executor)
    , m_acceptor(make_strand(executor))
    {
        beast::error_code ec;

        log("Opening acceptor.");
        ec = m_acceptor.open(endpoint.protocol(), ec);
        if (ec)
        {
            fail(ec, "m_acceptor.open");
            return;
        }

        log("Setting option reuse_address.");
        ec = m_acceptor.set_option(net::socket_base::reuse_address(true), ec);
        if (ec)
        {
            fail(ec, "m_acceptor.set_option");
            return;
        }

        log("Binding acceptor.");
        ec = m_acceptor.bind(endpoint, ec);
        if (ec)
        {
            fail(ec, "m_acceptor.bind");
            return;
        }

        log("Listening on acceptor.\n");
        ec = m_acceptor.listen(net::socket_base::max_listen_connections, ec);
        if (ec)
        {
            fail(ec, "m_acceptor.listen");
            return;
        }
    }

    void run()
    {
        do_accept();
    }

private:
    void do_accept()
    {
        m_acceptor.async_accept
        (
            make_strand(m_executor),
            beast::bind_front_handler
            (
                &listener::on_accept,
                shared_from_this()
            )
        );
    }

    void on_accept(beast::error_code const& ec, tcp::socket socket)
    {
        if (ec)
        {
            return fail(ec, "on_accept");
        }

        log("Accepted new connection.");

        std::make_shared<session>(std::move(socket))->run();

        // Continue accepting new connections.
        do_accept();
    }

    net::any_io_executor m_executor;
    tcp::acceptor m_acceptor;
};
