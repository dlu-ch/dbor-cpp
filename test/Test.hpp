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
        std::cout << "error: test failed on line " << __LINE__ << std::endl; \
        throw TestFailed(__LINE__); \
    }


#endif  // TEST_HPP_
