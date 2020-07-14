// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef TEST_HPP_
#define TEST_HPP_

#include <iostream>


#define ASSERT_EQUAL(expected, actual) \
    while ((expected) != (actual)) { \
        std::cout << __FILE__ << ":" << __LINE__ << ":error: test failed" << std::endl; \
        throw ::test::TestFailed(__LINE__); \
    }


#define ASSERT_TRUE(actual) \
    ASSERT_EQUAL(true, actual)


namespace test {

    struct TestFailed {
        TestFailed(std::size_t lineNo) : lineNo(lineNo) {}
        const std::size_t lineNo;
    };


    // Usage: ByteBufferBuilder{0x00, 0x12}.buffer
    struct ByteBufferBuilder {
        ByteBufferBuilder() : buffer(new std::uint8_t[1]), size(0) { *buffer = 0; }
        ByteBufferBuilder(std::initializer_list<std::uint8_t> bytes);
        virtual ~ByteBufferBuilder() noexcept { delete[] buffer; };

        uint8_t *const buffer;  // != nullptr
        const std::size_t size;
    };


    inline ByteBufferBuilder::ByteBufferBuilder(std::initializer_list<std::uint8_t> bytes)
        : buffer(new std::uint8_t[bytes.size()]), size(bytes.size())
    {
        std::uint8_t *p = buffer;
        for (std::uint8_t b: bytes)
            *p++ = b;
    }

}


#endif  // TEST_HPP_
