//
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

#ifndef BOOST_SOCKS_PROTO_IO_IMPL_CONNECT_V4_HPP
#define BOOST_SOCKS_PROTO_IO_IMPL_CONNECT_V4_HPP

#include <boost/socks_proto/detail/config.hpp>
#include <boost/core/allocator_access.hpp>
#include <boost/socks_proto/io/detail/connect_v4.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <string>

namespace boost {
namespace socks_proto {
namespace io {

// These functions are implementing what should
// be encapsulated into socks_proto::request
// in the future.
template <class SyncStream>
asio::ip::tcp::endpoint
connect_v4(
    SyncStream& stream,
    asio::ip::tcp::endpoint const& target_host,
    string_view socks_user,
    error_code& ec)
{
    // All these functions are repeatedly
    // implementing a pattern that should be
    // encapsulated into socks_proto::request
    // and socks_proto::reply in the future
    std::vector<unsigned char> buffer =
        detail::prepare_request_v4(
            target_host, socks_user);

    // Send a CONNECT request
    asio::write(
        stream,
        asio::buffer(buffer),
        ec);
    if (ec)
    {
        return asio::ip::tcp::endpoint{};
    }

    // Read the CONNECT reply
    buffer.resize(8);
    std::size_t n = asio::read(
        stream,
        asio::buffer(buffer.data(), buffer.size()),
        ec);
    if (ec == asio::error::eof)
    {
        // asio::error::eof indicates there was
        // a SOCKS error and the server
        // closed the connection cleanly
        return asio::ip::tcp::endpoint{};
    }
    else if (ec)
    {
        // read failed
        return asio::ip::tcp::endpoint{};
    }

    // Parse the CONNECT reply
    buffer.resize(n);
    auto r = detail::parse_reply_v4(buffer);
    if (!ec)
        ec = r.first;

    return r.second;
}

template <class SyncStream>
asio::ip::tcp::endpoint
connect_v4(
    SyncStream& stream,
    string_view target_host,
    std::uint16_t target_port,
    string_view socks_user,
    error_code& ec)
{
    // SOCKS4 does not support domain names
    // The domain name needs to be resolved
    // on the client
    asio::ip::tcp::resolver resolver{stream.get_executor()};
    asio::ip::tcp::resolver::results_type endpoints =
        resolver.resolve(
            std::string(target_host),
            std::to_string(target_port),
            ec);
    if (ec)
    {
        return asio::ip::tcp::endpoint{};
    }
    auto it = endpoints.begin();
    while (it != endpoints.end())
    {
        asio::ip::tcp::endpoint ep = it->endpoint();
        ++it;
        // SOCKS4 does not support IPv6 addresses
        if (ep.address().is_v6())
        {
            if (!ec)
                ec = asio::error::host_not_found;
            continue;
        }
        ep.port(target_port);
        ep = connect_v4(
            stream,
            ep,
            socks_user,
            ec);
        if (ec)
            continue;
        else
            return ep;
    }
    return asio::ip::tcp::endpoint{};
}

// SOCKS4 connect initiating function
// - These overloads look similar to what we
// should have in socks_io.
// - Their implementation includes what should
// be later encapsulated into
// socks_proto::request and socks_proto::reply.
template <class AsyncStream, class CompletionToken>
typename asio::async_result<
    typename asio::decay<CompletionToken>::type,
    void (error_code, asio::ip::tcp::endpoint)
>::return_type
async_connect_v4(
    AsyncStream& s,
    asio::ip::tcp::endpoint const& target_host,
    string_view socks_user,
    CompletionToken&& token)
{
    using DecayedToken =
        typename std::decay<CompletionToken>::type;
    using allocator_type =
        boost::allocator_rebind_t<
            typename asio::associated_allocator<
                DecayedToken>::type, unsigned char>;
    // async_initiate will:
    // - transform token into handler
    // - call initiation_fn(handler, args...)
    return asio::async_compose<
            CompletionToken,
            void (error_code, asio::ip::tcp::endpoint)>
    (
        // implementation of the composed asynchronous operation
        detail::connect_v4_implementation<
            AsyncStream, allocator_type>{
                s,
                target_host,
                socks_user,
                asio::get_associated_allocator(token)
            },
        // the completion token
        token,
        // I/O objects or I/O executors for which
        // outstanding work must be maintained
        s
    );
}

template <class AsyncStream, class CompletionToken>
typename asio::async_result<
    typename asio::decay<CompletionToken>::type,
    void (error_code, asio::ip::tcp::endpoint)
    >::return_type
async_connect_v4(
    AsyncStream& s,
    string_view app_domain,
    std::uint16_t app_port,
    string_view ident_id,
    CompletionToken&& token)
{
    // SOCKS4 does not support domain names
    // The domain name needs to be resolved
    // on the client
    using DecayedToken =
        typename std::decay<CompletionToken>::type;
    using allocator_type =
        boost::allocator_rebind_t<
            typename asio::associated_allocator<
                DecayedToken>::type, unsigned char>;
    // async_initiate will:
    // - transform token into handler
    // - call initiation_fn(handler, args...)
    return asio::async_compose<
            CompletionToken,
            void (error_code, asio::ip::tcp::endpoint)>
    (
        // implementation of the composed asynchronous operation
        detail::resolve_and_connect_v4_implementation<
            AsyncStream, allocator_type>{
                s,
                app_domain,
                app_port,
                ident_id,
                asio::get_associated_allocator(token)
            },
        // the completion token
        token,
        // I/O objects or I/O executors for which
        // outstanding work must be maintained
        s
    );
}

} // io
} // socks_proto
} // boost



#endif
