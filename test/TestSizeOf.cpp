// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "TestSizeOf.hpp"
#include "Test.hpp"
#include "Dbor/SizeOf.hpp"


static void testIntegerFunc() {
    // unsigned
    static_assert(dbor::SizeOf::integer(0u) == 1u, "");
    static_assert(dbor::SizeOf::integer(1u) == 1u, "");

    static_assert(dbor::SizeOf::integer(0x117u) == 2u, "");
    static_assert(dbor::SizeOf::integer(0x118u) == 3u, "");

    static_assert(dbor::SizeOf::integer(0x10117ul) == 3u, "");
    static_assert(dbor::SizeOf::integer(0x10118ul) == 4u, "");
    static_assert(dbor::SizeOf::integer(0x1010117ul) == 4u, "");
    static_assert(dbor::SizeOf::integer(0x1010118ul) == 5u, "");

    static_assert(dbor::SizeOf::integer(0x101010117ull) == 5u, "");
    static_assert(dbor::SizeOf::integer(0x101010118ull) == 6u, "");
    static_assert(dbor::SizeOf::integer(0x10101010117ull) == 6u, "");
    static_assert(dbor::SizeOf::integer(0x10101010118ull) == 7u, "");
    static_assert(dbor::SizeOf::integer(0x1010101010117ull) == 7u, "");
    static_assert(dbor::SizeOf::integer(0x1010101010118ull) == 8u, "");
    static_assert(dbor::SizeOf::integer(0x101010101010117ull) == 8u, "");
    static_assert(dbor::SizeOf::integer(0x101010101010118ull) == 9u, "");

    static_assert(dbor::SizeOf::integer(std::numeric_limits<std::uint64_t>::max()) == 9u, "");

    // signed
    static_assert(dbor::SizeOf::integer(0) == 1u, "");
    static_assert(dbor::SizeOf::integer(1) == 1u, "");

    static_assert(dbor::SizeOf::integer(0x117) == 2u, "");
    static_assert(dbor::SizeOf::integer(0x118) == 3u, "");
    static_assert(dbor::SizeOf::integer(-0x118) == 2u, "");
    static_assert(dbor::SizeOf::integer(-0x119) == 3u, "");

    static_assert(dbor::SizeOf::integer(-0x10118l) == 3u, "");
    static_assert(dbor::SizeOf::integer(-0x10119l) == 4u, "");
    static_assert(dbor::SizeOf::integer(-0x1010118l) == 4u, "");
    static_assert(dbor::SizeOf::integer(-0x1010119l) == 5u, "");

    static_assert(dbor::SizeOf::integer(-0x101010118ll) == 5u, "");
    static_assert(dbor::SizeOf::integer(-0x101010119ll) == 6u, "");
    static_assert(dbor::SizeOf::integer(-0x10101010118ll) == 6u, "");
    static_assert(dbor::SizeOf::integer(-0x10101010119ll) == 7u, "");
    static_assert(dbor::SizeOf::integer(-0x1010101010118ll) == 7u, "");
    static_assert(dbor::SizeOf::integer(-0x1010101010119ll) == 8u, "");
    static_assert(dbor::SizeOf::integer(-0x101010101010118ll) == 8u, "");
    static_assert(dbor::SizeOf::integer(-0x101010101010119ll) == 9u, "");

    static_assert(dbor::SizeOf::integer(std::numeric_limits<std::int64_t>::min()) == 9u, "");
    static_assert(dbor::SizeOf::integer(std::numeric_limits<std::int64_t>::max()) == 9u, "");
}


static void testIntegerStruct() {
    // unsigned
    static_assert(dbor::SizeOf::Integer<std::uint8_t>::value == 2u, "");
    static_assert(dbor::SizeOf::Integer<std::uint16_t>::value == 3u, "");
    static_assert(dbor::SizeOf::Integer<std::uint32_t>::value == 5u, "");
    static_assert(dbor::SizeOf::Integer<std::uint64_t>::value == 9u, "");

    // signed
    static_assert(dbor::SizeOf::Integer<std::int8_t>::value == 2u, "");
    static_assert(dbor::SizeOf::Integer<std::int16_t>::value == 3u, "");
    static_assert(dbor::SizeOf::Integer<std::int32_t>::value == 5u, "");
    static_assert(dbor::SizeOf::Integer<std::int64_t>::value == 9u, "");
}


static void testByteStringFunc() {
    static_assert(dbor::SizeOf::byteString(0) == 1u, "");
    static_assert(dbor::SizeOf::byteString(23) == 1u + 23u, "");
    static_assert(dbor::SizeOf::byteString(24) == 2u + 24u, "");
    static_assert(dbor::SizeOf::byteString(std::numeric_limits<std::size_t>::max())
                  == std::numeric_limits<std::size_t>::max(), "");

    static_assert(dbor::SizeOf::byteString({}) == 1u, "");
    static_assert(dbor::SizeOf::byteString({1, 2, 3, 4}) == 5u, "");
}


static void testUtf8StringFunc() {
    static_assert(dbor::SizeOf::utf8String(24) == 2u + 24u, "");
    static_assert(dbor::SizeOf::utf8String(std::numeric_limits<std::size_t>::max())
                  == std::numeric_limits<std::size_t>::max(), "");

    static_assert(dbor::SizeOf::utf8String(nullptr) == 1u, "");
    static_assert(dbor::SizeOf::utf8String("") == 1u, "");
    static_assert(dbor::SizeOf::utf8String("123") == 1u + 3u, "");

    static_assert(dbor::SizeOf::utf8String(24) == 2u + 24u, "");
}


static void testAddFunc() {
    static_assert(dbor::SizeOf::addSaturating(42, 23) == 42 + 23, "");
    static_assert(dbor::SizeOf::addSaturating(std::numeric_limits<std::size_t>::max(), 23)
                  == std::numeric_limits<std::size_t>::max(), "");
}


static void testAddStruct() {
    static_assert(dbor::SizeOf::Add<42>::value == 42, "");
    static_assert(dbor::SizeOf::Add<42, 23>::value == 42 + 23, "");
    static_assert(dbor::SizeOf::Add<42, 23, 1, 0, 7>::value == 42 + 23 + 1 + 0 + 7, "");
}


void testSizeOf() {
    testIntegerStruct();
    testIntegerFunc();
    testByteStringFunc();
    testUtf8StringFunc();
    testAddFunc();
    testAddStruct();
}
