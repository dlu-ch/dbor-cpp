// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "TestValue.hpp"
#include "Test.hpp"
#include "Dbor/Value.hpp"


static void testDefaultConstructedIsEmpty() {
    dbor::Value v;
    ASSERT_EQUAL(nullptr, v.buffer());
    ASSERT_EQUAL(0u, v.size());
}


static void testIsEmptyWithoutBuffer() {
    {
        dbor::Value v(nullptr, 27);
        ASSERT_EQUAL(nullptr, v.buffer());
        ASSERT_EQUAL(0u, v.size());
    }
    {
        const std::uint8_t buffer[1] = {};
        dbor::Value v(buffer, 0);
        ASSERT_EQUAL(nullptr, v.buffer());
        ASSERT_EQUAL(0u, v.size());
    }
}


static void testSizeOfIncompleteIsCapacity() {
   const std::uint8_t buffer[] = { 0x3F, 0x00 };
   dbor::Value v(buffer, sizeof(buffer));
   ASSERT_EQUAL(buffer, v.buffer());
   ASSERT_EQUAL(sizeof(buffer), v.size());
}


static void testSizeOfMultipleIsSizeOfFirst() {
   const std::uint8_t buffer[] = { 0x19, 0x01, 0x02, 0xFF };
   dbor::Value v(buffer, sizeof(buffer));
   ASSERT_EQUAL(buffer, v.buffer());
   ASSERT_EQUAL(3, v.size());
}


void testValue() {
    testDefaultConstructedIsEmpty();
    testIsEmptyWithoutBuffer();
    testSizeOfIncompleteIsCapacity();
    testSizeOfMultipleIsSizeOfFirst();
}
