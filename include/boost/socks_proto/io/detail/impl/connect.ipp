//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#ifndef BOOST_SOCKS_PROTO_IO_DETAIL_IMPL_CONNECT_IPP
#define BOOST_SOCKS_PROTO_IO_DETAIL_IMPL_CONNECT_IPP

#include <boost/socks_proto/io/detail/connect.hpp>

namespace boost {
namespace socks_proto {
namespace io {
namespace detail {

endpoint
parse_reply_v5(
    unsigned char const* buffer,
    std::size_t n,
    error_code& ec)
{
    if (n < 2)
    {
        // Successful messages have size 8
        // Some servers return only 2 bytes,
        // since DSTPORT and DSTIP can be ignored
        // in SOCKS4 or to return error messages,
        // including SOCKS5 errors
        ec = asio::error::access_denied;
        return {};
    }

    // VER:
    // In SOCKS5, the reply version is allowed to
    // be 0x00. In general, this is the SOCKS version as
    // 40.
    if (buffer[0] != 0x05)
    {
        ec = asio::error::no_protocol_option;
        return {};
    }

    // REP: the res
    ec = static_cast<error>(buffer[1]);
    if (ec != condition::succeeded)
        return {};

    // According to the RFCs, DSTPORT and DSTIP
    // should be ignored in SOCKS4 and must not
    // be ignored in SOCKS5.
    // In practice, DSTPORT and DSTIP are also
    // ignored by SOCKS5 servers, and the
    // reply is filled with 0x00s.
    // When this happends, the original
    // destination endpoint is considered to
    // be DSTPORT and DSTIP.
    if (n < 10)
        return {};

    // ATYP
    address_type atyp = to_address_type(buffer[3]);

    // DSTIP
    switch (atyp)
    {
    case address_type::ip_v4:
    {
        if (n < 10)
        {
            return {};
        }
        std::uint32_t ip{ buffer[4] };
        ip = (ip << 8) | buffer[5];
        ip = (ip << 8) | buffer[6];
        ip = (ip << 8) | buffer[7];
        std::uint16_t port{ buffer[8] };
        port <<= 8;
        port |= buffer[9];
        return endpoint{
            asio::ip::make_address_v4(ip),
            port
        };
    }
    case address_type::ip_v6:
    {
        if (n < 22)
        {
            return {};
        }
        asio::ip::address_v6::bytes_type ip;
        std::size_t i = 0;
        std::memcpy(ip.data(), buffer + 4, 16);
        std::uint16_t port{ buffer[i++] };
        port <<= 8;
        port |= buffer[i++];
        return endpoint{
            asio::ip::make_address_v6(ip),
            port
        };
    }
    default:
        ec = error::general_failure;
        return {};
    }
}

} // detail
} // io
} // socks_proto
} // boost

#endif
