//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2022 Alan de Freitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/alandefreitas/socks_proto
//

// Test that header file is self-contained.
#include <boost/socks_proto/reply_code.hpp>

#include "test_suite.hpp"

namespace boost {
namespace socks_proto {

class reply_code_test
{
public:
    void
    testReplyCode()
    {
        auto const check = [&](reply_code c, int i)
            {
                BOOST_TEST(to_reply_code(i) == c);
            };
        check(reply_code::succeeded, 0x00);
        check(reply_code::general_failure, 0x01);
        check(reply_code::connection_not_allowed_by_ruleset, 0x02);
        check(reply_code::network_unreachable, 0x03);
        check(reply_code::host_unreachable, 0x04);
        check(reply_code::connection_refused, 0x05);
        check(reply_code::ttl_expired, 0x06);
        check(reply_code::command_not_supported, 0x07);
        check(reply_code::address_type_not_supported, 0x08);
        check(reply_code::unassigned, 0xFF);

        BOOST_TEST(to_reply_code(0x09) == reply_code::unassigned);

        auto const good =
            [&](reply_code v)
            {
                BOOST_TEST_NE(to_string(v), "Unassigned");
                std::stringstream ss;
                ss << v;
                BOOST_TEST_EQ(to_string(v), ss.str());
            };
        good(reply_code::succeeded);
        good(reply_code::general_failure);
        good(reply_code::connection_not_allowed_by_ruleset);
        good(reply_code::network_unreachable);
        good(reply_code::host_unreachable);
        good(reply_code::connection_refused);
        good(reply_code::ttl_expired);
        good(reply_code::command_not_supported);
        good(reply_code::address_type_not_supported);

        auto const bad =
            [&](reply_code v)
        {
            BOOST_TEST(to_string(v) == "Unassigned");
        };
        bad(reply_code::unassigned);
        bad(static_cast<reply_code>(0xFE));
    }

    void
    run()
    {
        testReplyCode();
    }
};

TEST_SUITE(reply_code_test, "boost.socks_proto.reply_code");

} // socks_proto
} // boost

