// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef TEST_HPP_
#define TEST_HPP_

#include <iostream>


struct TestFailed {
    TestFailed(std::size_t lineNo) : lineNo(lineNo) {}
    const std::size_t lineNo;
};


#define ASSERT_EQUAL(expected, actual) \
    while ((expected) != (actual)) { \
        std::cout << __FILE__ << ":" << __LINE__ << ":error: test failed" << std::endl; \
        throw TestFailed(__LINE__); \
    }


#define ASSERT_TRUE(actual) \
    ASSERT_EQUAL(true, actual)


#endif  // TEST_HPP_
