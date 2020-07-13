// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "TestValue.hpp"
#include <initializer_list>
#include "Test.hpp"
#include "Dbor/Value.hpp"

// Usage: ValueBuilder{0x00, 0x12}.value
struct ValueBuilder: public test::ByteBufferBuilder {
    ValueBuilder(std::initializer_list<std::uint8_t> bytes)
        : test::ByteBufferBuilder(bytes)
        , value(buffer_, bytes.size())
    {
    }

    const dbor::Value value;
};


static void testDefaultConstructedIsEmpty() {
    dbor::Value v;
    ASSERT_EQUAL(nullptr, v.buffer());
    ASSERT_EQUAL(0u, v.size());
    ASSERT_TRUE(!v.isComplete());
}


static void testIsEmptyWithoutBuffer() {
    {
        dbor::Value v(nullptr, 27);
        ASSERT_EQUAL(nullptr, v.buffer());
        ASSERT_EQUAL(0u, v.size());
        ASSERT_TRUE(!v.isComplete());
    }
    {
        const std::uint8_t buffer[1] = {};
        dbor::Value v(buffer, 0);
        ASSERT_EQUAL(nullptr, v.buffer());
        ASSERT_EQUAL(0u, v.size());
        ASSERT_TRUE(!v.isComplete());
    }
}


static void testSizeOfIncompleteIsCapacity() {
   const std::uint8_t buffer[] = {0x3F, 0x00};
   dbor::Value v(buffer, sizeof(buffer));
   ASSERT_EQUAL(buffer, v.buffer());
   ASSERT_EQUAL(sizeof(buffer), v.size());
   ASSERT_TRUE(!v.isComplete());
}


static void testSizeOfMultipleIsSizeOfFirst() {
   const std::uint8_t buffer[] = {0x19, 0x01, 0x02, 0xFF};
   dbor::Value v(buffer, sizeof(buffer));
   ASSERT_EQUAL(buffer, v.buffer());
   ASSERT_EQUAL(3, v.size());
   ASSERT_TRUE(v.isComplete());
}


static void testGetIntegerUint8() {
    std::uint8_t v;

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, dbor::Value().get(v));
    ASSERT_EQUAL(0, v);

    // IntegerValue(v) with v >= 0
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x00}.value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x17}.value.get(v));
    ASSERT_EQUAL(23, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x18, 0x00}).value.get(v));
    ASSERT_EQUAL(24, v);
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, ValueBuilder{0x18}.value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x18, 0xE7}).value.get(v));
    ASSERT_EQUAL(UINT8_MAX, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, (ValueBuilder{0x18, 0xFF}).value.get(v));
    ASSERT_EQUAL(UINT8_MAX, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}).value.get(v));
    ASSERT_EQUAL(UINT8_MAX, v);

    // IntegerValue(v) with v < 0
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0x20}.value.get(v));
    ASSERT_EQUAL(0, v);

    // MinusZeroValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_IMPRECISE, ValueBuilder{0xFC}.value.get(v));
    ASSERT_EQUAL(0, v);

    // MinusInfinityValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFD}.value.get(v));
    ASSERT_EQUAL(0, v);

    // InfinityValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFE}.value.get(v));
    ASSERT_EQUAL(UINT8_MAX, v);

    // NoneValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::NO_OBJECT, ValueBuilder{0xFF}.value.get(v));
    ASSERT_EQUAL(0, v);

    // TODO other types
}


static void testGetIntegerUint16() {
    std::uint16_t v;

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, dbor::Value().get(v));
    ASSERT_EQUAL(0, v);

    // IntegerValue(v) with v >= 0
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x00}.value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x17}.value.get(v));
    ASSERT_EQUAL(23, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x18, 0xE8}).value.get(v));
    ASSERT_EQUAL(0x100, v);
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, ValueBuilder{0x18}.value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x19, 0x00, 0x00}).value.get(v));
    ASSERT_EQUAL(0x118, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x19, 0xE7, 0xFE}).value.get(v));
    ASSERT_EQUAL(UINT16_MAX, v);
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, (ValueBuilder{0x19, 0xE7}).value.get(v));
    ASSERT_EQUAL(0, v);


    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, (ValueBuilder{0x19, 0xFF, 0xFF}).value.get(v));
    ASSERT_EQUAL(UINT16_MAX, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}).value.get(v));
    ASSERT_EQUAL(UINT16_MAX, v);

    // IntegerValue(v) with v < 0
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0x20}.value.get(v));
    ASSERT_EQUAL(0, v);

    // MinusZeroValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_IMPRECISE, ValueBuilder{0xFC}.value.get(v));
    ASSERT_EQUAL(0, v);

    // MinusInfinityValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFD}.value.get(v));
    ASSERT_EQUAL(0, v);

    // InfinityValue
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFE}.value.get(v));
    ASSERT_EQUAL(UINT16_MAX, v);

    // NoneValue
    ASSERT_EQUAL(dbor::ResultCodes::NO_OBJECT, ValueBuilder{0xFF}.value.get(v));
    ASSERT_EQUAL(0, v);

    // TODO other types
}


static void testGetIntegerUint32() {
    std::uint32_t v;

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, dbor::Value().get(v));
    ASSERT_EQUAL(0, v);

    // IntegerValue(v) with v >= 0
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x00}.value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x17}.value.get(v));
    ASSERT_EQUAL(23, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x19, 0xE8, 0xFE}).value.get(v));
    ASSERT_EQUAL(0x10000, v);
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, (ValueBuilder{0x19, 0xE8}).value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK,
                 (ValueBuilder{0x1B, 0x00, 0x00, 0x00, 0x00}).value.get(v));
    ASSERT_EQUAL(0x1010118, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK,
                 (ValueBuilder{0x1B, 0xE7, 0xFE, 0xFE, 0xFE}).value.get(v));
    ASSERT_EQUAL(UINT32_MAX, v);
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE,
                 (ValueBuilder{0x1B, 0xE7, 0xFE, 0xFE}).value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x1B, 0xFF, 0xFF, 0xFF, 0xFF}).value.get(v));
    ASSERT_EQUAL(UINT32_MAX, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}).value.get(v));
    ASSERT_EQUAL(UINT32_MAX, v);

    // IntegerValue(v) with v < 0
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0x20}.value.get(v));
    ASSERT_EQUAL(0, v);

    // MinusZeroValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_IMPRECISE, ValueBuilder{0xFC}.value.get(v));
    ASSERT_EQUAL(0, v);

    // MinusInfinityValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFD}.value.get(v));
    ASSERT_EQUAL(0, v);

    // InfinityValue
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFE}.value.get(v));
    ASSERT_EQUAL(UINT32_MAX, v);

    // NoneValue
    ASSERT_EQUAL(dbor::ResultCodes::NO_OBJECT, ValueBuilder{0xFF}.value.get(v));
    ASSERT_EQUAL(0, v);

    // TODO other types
}


static void testGetIntegerUint64() {
    std::uint64_t v;

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, dbor::Value().get(v));
    ASSERT_EQUAL(0, v);

    // IntegerValue(v) with v >= 0
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x00}.value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x17}.value.get(v));
    ASSERT_EQUAL(23, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK,
                 (ValueBuilder{0x1B, 0xE8, 0xFE, 0xFE, 0xFE}).value.get(v));
    ASSERT_EQUAL(0x100000000ul, v);
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE,
                 (ValueBuilder{0x1B, 0xE8, 0xFE, 0xFE}).value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK,
                 (ValueBuilder{0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}).value.get(v));
    ASSERT_EQUAL(0x101010101010118ull, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK,
                 (ValueBuilder{0x1F, 0xE7, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE}).value.get(v));
    ASSERT_EQUAL(UINT64_MAX, v);
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE,
                 (ValueBuilder{0x1F, 0xE7, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE}).value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}).value.get(v));
    ASSERT_EQUAL(UINT64_MAX, v);

    // IntegerValue(v) with v < 0
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0x20}.value.get(v));
    ASSERT_EQUAL(0, v);

    // MinusZeroValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_IMPRECISE, ValueBuilder{0xFC}.value.get(v));
    ASSERT_EQUAL(0, v);

    // MinusInfinityValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFD}.value.get(v));
    ASSERT_EQUAL(0, v);

    // InfinityValue
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFE}.value.get(v));
    ASSERT_EQUAL(UINT64_MAX, v);

    // NoneValue
    ASSERT_EQUAL(dbor::ResultCodes::NO_OBJECT, ValueBuilder{0xFF}.value.get(v));
    ASSERT_EQUAL(0, v);

    // TODO other types
}


static void testGetIntegerInt8() {
    static_assert(INT8_MIN == -128 && INT8_MAX == 127, "expect 2's complement representation");

    std::int8_t v;

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, dbor::Value().get(v));
    ASSERT_EQUAL(0, v);

    // IntegerValue(v) with v >= 0
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x00}.value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x17}.value.get(v));
    ASSERT_EQUAL(23, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x18, 0x00}).value.get(v));
    ASSERT_EQUAL(24, v);
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, ValueBuilder{0x18}.value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x18, 0x67}).value.get(v));
    ASSERT_EQUAL(INT8_MAX, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, (ValueBuilder{0x18, 0x68}).value.get(v));
    ASSERT_EQUAL(INT8_MAX, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}).value.get(v));
    ASSERT_EQUAL(INT8_MAX, v);

    // IntegerValue(v) with v < 0
    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x20}.value.get(v));
    ASSERT_EQUAL(-1, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x37}.value.get(v));
    ASSERT_EQUAL(-24, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x38, 0x00}).value.get(v));
    ASSERT_EQUAL(-25, v);
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, ValueBuilder{0x38}.value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x38, 0x67}).value.get(v));
    ASSERT_EQUAL(INT8_MIN, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, (ValueBuilder{0x38, 0x68}).value.get(v));
    ASSERT_EQUAL(INT8_MIN, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}).value.get(v));
    ASSERT_EQUAL(INT8_MIN, v);

    // MinusZeroValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_IMPRECISE, ValueBuilder{0xFC}.value.get(v));
    ASSERT_EQUAL(0, v);

    // MinusInfinityValue
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFD}.value.get(v));
    ASSERT_EQUAL(INT8_MIN, v);

    // InfinityValue
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFE}.value.get(v));
    ASSERT_EQUAL(INT8_MAX, v);

    // NoneValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::NO_OBJECT, ValueBuilder{0xFF}.value.get(v));
    ASSERT_EQUAL(0, v);

    // TODO other types
}


static void testGetIntegerInt16() {
    static_assert(INT16_MIN == -32768 && INT16_MAX == 32767,
                  "expect 2's complement representation");

    std::int16_t v;

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, dbor::Value().get(v));
    ASSERT_EQUAL(0, v);

    // IntegerValue(v) with v >= 0
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x00}.value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x17}.value.get(v));
    ASSERT_EQUAL(23, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x18, 0x68}).value.get(v));
    ASSERT_EQUAL(128, v);
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, ValueBuilder{0x18}.value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x19, 0xE7, 0x7E}).value.get(v));
    ASSERT_EQUAL(INT16_MAX, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, (ValueBuilder{0x19, 0xE8, 0x7E}).value.get(v));
    ASSERT_EQUAL(INT16_MAX, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}).value.get(v));
    ASSERT_EQUAL(INT16_MAX, v);

    // IntegerValue(v) with v < 0
    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x20}.value.get(v));
    ASSERT_EQUAL(-1, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x37}.value.get(v));
    ASSERT_EQUAL(-24, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x38, 0x68}).value.get(v));
    ASSERT_EQUAL(-129, v);
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, ValueBuilder{0x38}.value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x39, 0xE7, 0x7E}).value.get(v));
    ASSERT_EQUAL(INT16_MIN, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, (ValueBuilder{0x39, 0xE8, 0x7E}).value.get(v));
    ASSERT_EQUAL(INT16_MIN, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}).value.get(v));
    ASSERT_EQUAL(INT16_MIN, v);

    // MinusZeroValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_IMPRECISE, ValueBuilder{0xFC}.value.get(v));
    ASSERT_EQUAL(0, v);

    // MinusInfinityValue
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFD}.value.get(v));
    ASSERT_EQUAL(INT16_MIN, v);

    // InfinityValue
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFE}.value.get(v));
    ASSERT_EQUAL(INT16_MAX, v);

    // NoneValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::NO_OBJECT, ValueBuilder{0xFF}.value.get(v));
    ASSERT_EQUAL(0, v);

    // TODO other types
}


static void testGetIntegerInt32() {
    static_assert(INT32_MIN == -2147483648l && INT32_MAX == 2147483647l,
                  "expect 2's complement representation");

    std::int32_t v;

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, dbor::Value().get(v));
    ASSERT_EQUAL(0, v);

    // IntegerValue(v) with v >= 0
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x00}.value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x17}.value.get(v));
    ASSERT_EQUAL(23, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x19, 0xE8, 0x7E}).value.get(v));
    ASSERT_EQUAL(32768, v);
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, (ValueBuilder{0x19, 0xE8}).value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x1B, 0xE7, 0xFE, 0xFE, 0x7E}).value.get(v));
    ASSERT_EQUAL(INT32_MAX, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x1B, 0xE8, 0xFE, 0xFE, 0x7E}).value.get(v));
    ASSERT_EQUAL(INT32_MAX, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}).value.get(v));
    ASSERT_EQUAL(INT32_MAX, v);

    // IntegerValue(v) with v < 0
    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x20}.value.get(v));
    ASSERT_EQUAL(-1, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x37}.value.get(v));
    ASSERT_EQUAL(-24, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x39, 0xE8, 0x7E}).value.get(v));
    ASSERT_EQUAL(-32769, v);
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, ValueBuilder{0x38}.value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x3B, 0xE7, 0xFE, 0xFE, 0x7E}).value.get(v));
    ASSERT_EQUAL(INT32_MIN, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x3B, 0xE8, 0xFE, 0xFE, 0x7E}).value.get(v));
    ASSERT_EQUAL(INT32_MIN, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}).value.get(v));
    ASSERT_EQUAL(INT32_MIN, v);

    // MinusZeroValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_IMPRECISE, ValueBuilder{0xFC}.value.get(v));
    ASSERT_EQUAL(0, v);

    // MinusInfinityValue
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFD}.value.get(v));
    ASSERT_EQUAL(INT32_MIN, v);

    // InfinityValue
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFE}.value.get(v));
    ASSERT_EQUAL(INT32_MAX, v);

    // NoneValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::NO_OBJECT, ValueBuilder{0xFF}.value.get(v));
    ASSERT_EQUAL(0, v);

    // TODO other types
}


static void testGetIntegerInt64() {
    static_assert(INT64_MAX == 9223372036854775807ll, "expect 2's complement representation");

    std::int64_t v;

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, dbor::Value().get(v));
    ASSERT_EQUAL(0, v);

    // IntegerValue(v) with v >= 0
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x00}.value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x17}.value.get(v));
    ASSERT_EQUAL(23, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x1B, 0xE8, 0xFE, 0xFE, 0x7E}).value.get(v));
    ASSERT_EQUAL(2147483648ll, v);
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE,
                 (ValueBuilder{0x1B, 0xE8, 0xFE, 0xFE}).value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK,
                 (ValueBuilder{0x1F, 0xE7, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0x7E}).value.get(v));
    ASSERT_EQUAL(INT64_MAX, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x1F, 0xE8, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0x7E}).value.get(v));
    ASSERT_EQUAL(INT64_MAX, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}).value.get(v));
    ASSERT_EQUAL(INT64_MAX, v);

    // IntegerValue(v) with v < 0
    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x20}.value.get(v));
    ASSERT_EQUAL(-1, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x37}.value.get(v));
    ASSERT_EQUAL(-24, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK, (ValueBuilder{0x3B, 0xE8, 0xFE, 0xFE, 0x7E}).value.get(v));
    ASSERT_EQUAL(-2147483649ll, v);
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE,
                 (ValueBuilder{0x3B, 0xE8, 0xFE, 0xFE}).value.get(v));
    ASSERT_EQUAL(0, v);

    ASSERT_EQUAL(dbor::ResultCodes::OK,
                 (ValueBuilder{0x3F, 0xE7, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0x7E}).value.get(v));
    ASSERT_EQUAL(INT64_MIN, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x3F, 0xE8, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0x7E}).value.get(v));
    ASSERT_EQUAL(INT64_MIN, v);

    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                 (ValueBuilder{0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}).value.get(v));
    ASSERT_EQUAL(INT64_MIN, v);

    // MinusZeroValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_IMPRECISE, ValueBuilder{0xFC}.value.get(v));
    ASSERT_EQUAL(0, v);

    // MinusInfinityValue
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFD}.value.get(v));
    ASSERT_EQUAL(INT64_MIN, v);

    // InfinityValue
    ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME, ValueBuilder{0xFE}.value.get(v));
    ASSERT_EQUAL(INT64_MAX, v);

    // NoneValue
    v = 7;
    ASSERT_EQUAL(dbor::ResultCodes::NO_OBJECT, ValueBuilder{0xFF}.value.get(v));
    ASSERT_EQUAL(0, v);

    // TODO other types
}


static void testGetByteString() {
    static const std::uint8_t ZERO = 0;
    const std::uint8_t *p;
    std::size_t n;

    p = &ZERO;
    n = 7;
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, dbor::Value().get(p, n));
    ASSERT_EQUAL(nullptr, p);
    ASSERT_EQUAL(0, n);

    p = &ZERO;
    n = 7;
    {
        const std::uint8_t buffer[] = {0x40};
        ASSERT_EQUAL(dbor::ResultCodes::OK, dbor::Value(buffer, sizeof(buffer)).get(p, n));
        ASSERT_EQUAL(buffer + 1, p);
        ASSERT_EQUAL(0, n);
    }

    p = &ZERO;
    n = 7;
    {
        const std::uint8_t buffer[] = {0x43, 0x01, 0x02, 0x03};
        ASSERT_EQUAL(dbor::ResultCodes::OK, dbor::Value(buffer, sizeof(buffer)).get(p, n));
        ASSERT_EQUAL(buffer + 1, p);
        ASSERT_EQUAL(3, n);

        ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, dbor::Value(buffer, sizeof(buffer) - 1).get(p, n));
        ASSERT_EQUAL(nullptr, p);
        ASSERT_EQUAL(0, n);
    }

    p = &ZERO;
    n = 7;
    {
        const std::uint8_t buffer[2 + 24] = {0x58, 0x00};
        ASSERT_EQUAL(dbor::ResultCodes::OK, dbor::Value(buffer, sizeof(buffer)).get(p, n));
        ASSERT_EQUAL(buffer + 2, p);
        ASSERT_EQUAL(24, n);

        ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE,
                     dbor::Value(buffer, sizeof(buffer) - 1).get(p, n));
        ASSERT_EQUAL(nullptr, p);
        ASSERT_EQUAL(0, n);
    }

    // NoneValue
    p = &ZERO;
    n = 7;
    ASSERT_EQUAL(dbor::ResultCodes::NO_OBJECT, ValueBuilder{0xFF}.value.get(p, n));
    ASSERT_EQUAL(nullptr, p);
    ASSERT_EQUAL(0, n);

    // IntegerValue
    p = &ZERO;
    n = 7;
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPATIBLE, ValueBuilder{0x00}.value.get(p, n));
    ASSERT_EQUAL(nullptr, p);
    ASSERT_EQUAL(0, n);

    // Utf8StringValue
    p = &ZERO;
    n = 7;
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPATIBLE, ValueBuilder{0x60}.value.get(p, n));
    ASSERT_EQUAL(nullptr, p);
    ASSERT_EQUAL(0, n);
}


static void testGetUtf8String() {
    dbor::String s;

    ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE, dbor::Value().get(s, 10));
    ASSERT_EQUAL(0u, s.size());

    ASSERT_EQUAL(dbor::ResultCodes::OK, ValueBuilder{0x60}.value.get(s, 100));
    ASSERT_EQUAL(0, s.size());

    {
        const std::uint8_t buffer[] = {0x67, 0x01, 0xF0, 0x90, 0x80, 0x80, 0x02, 0x03};
        ASSERT_EQUAL(dbor::ResultCodes::OK, dbor::Value(buffer, sizeof(buffer)).get(s, 100));
        ASSERT_EQUAL(buffer + 1, s.buffer());
        ASSERT_EQUAL(7, s.size());

        ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE,
                     dbor::Value(buffer, sizeof(buffer) - 1).get(s, 100));
        ASSERT_EQUAL(0u, s.size());
    }

    {
        const std::uint8_t buffer[2 + 24] = {0x78, 0x00};
        ASSERT_EQUAL(dbor::ResultCodes::OK, dbor::Value(buffer, sizeof(buffer)).get(s, 24));
        ASSERT_EQUAL(buffer + 2, s.buffer());
        ASSERT_EQUAL(24, s.size());

        ASSERT_EQUAL(dbor::ResultCodes::INCOMPLETE,
                     dbor::Value(buffer, sizeof(buffer) - 1).get(s, 100));
        ASSERT_EQUAL(0u, s.size());
    }

    // truncated well-formed

    {
        const std::uint8_t buffer[] = {
            0x6A,
            0x20,
            0xC2, 0x80,
            0xF0, 0x90, 0x80, 0x80,
            0xED, 0x9F, 0xBF
        };

        ASSERT_EQUAL(dbor::ResultCodes::OK, dbor::Value(buffer, sizeof(buffer)).get(s, 10));
        ASSERT_EQUAL(buffer + 1, s.buffer());
        ASSERT_EQUAL(10, s.size());

        ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                     dbor::Value(buffer, sizeof(buffer)).get(s, 9));
        ASSERT_EQUAL(buffer + 1, s.buffer());
        ASSERT_EQUAL(7, s.size());

        ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                     dbor::Value(buffer, sizeof(buffer)).get(s, 8));
        ASSERT_EQUAL(7, s.size());

        ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                     dbor::Value(buffer, sizeof(buffer)).get(s, 7));
        ASSERT_EQUAL(7, s.size());

        ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                     dbor::Value(buffer, sizeof(buffer)).get(s, 6));
        ASSERT_EQUAL(3, s.size());

        ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                     dbor::Value(buffer, sizeof(buffer)).get(s, 4));
        ASSERT_EQUAL(3, s.size());

        ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                     dbor::Value(buffer, sizeof(buffer)).get(s, 2));
        ASSERT_EQUAL(1, s.size());

        ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                     dbor::Value(buffer, sizeof(buffer)).get(s, 0));
        ASSERT_EQUAL(0, s.size());
    }

    // truncated ill-formed

    {
        const std::uint8_t buffer[] = {
            0x68,
            0xF4, 0x8F, 0xBF, 0x00,
            0xF4, 0x90, 0x80, 0x80
        };

        ASSERT_EQUAL(dbor::ResultCodes::OK, dbor::Value(buffer, sizeof(buffer)).get(s, 8));
        ASSERT_EQUAL(buffer + 1, s.buffer());
        ASSERT_EQUAL(8, s.size());
        ASSERT_EQUAL(dbor::ResultCodes::ILLFORMED, s.check());

        ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                     dbor::Value(buffer, sizeof(buffer)).get(s, 7));
        ASSERT_EQUAL(buffer + 1, s.buffer());
        ASSERT_EQUAL(4, s.size());

        ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                     dbor::Value(buffer, sizeof(buffer)).get(s, 3));
        ASSERT_EQUAL(3, s.size());

        ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                     dbor::Value(buffer, sizeof(buffer)).get(s, 3));
        ASSERT_EQUAL(3, s.size());

        ASSERT_EQUAL(dbor::ResultCodes::APPROX_EXTREME,
                     dbor::Value(buffer, sizeof(buffer)).get(s, 2));
        ASSERT_EQUAL(0, s.size());
    }

    // NoneValue
    ASSERT_EQUAL(dbor::ResultCodes::NO_OBJECT, ValueBuilder{0xFF}.value.get(s, 100));
    ASSERT_EQUAL(0u, s.size());

    // IntegerValue
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPATIBLE, ValueBuilder{0x00}.value.get(s, 100));
    ASSERT_EQUAL(0u, s.size());

    // ByteStringValue
    ASSERT_EQUAL(dbor::ResultCodes::INCOMPATIBLE, ValueBuilder{0x40}.value.get(s, 100));
    ASSERT_EQUAL(0u, s.size());
}


static void testIsNone() {
    ASSERT_TRUE(ValueBuilder{0xFF}.value.isNone());

    ASSERT_TRUE(!dbor::Value().isNone());
    ASSERT_TRUE(!ValueBuilder{0xFE}.value.isNone());
}


static void testIsNumberlike() {
    ASSERT_TRUE(ValueBuilder{0xFC}.value.isNumberlike());
    ASSERT_TRUE(ValueBuilder{0xFD}.value.isNumberlike());
    ASSERT_TRUE(ValueBuilder{0xFE}.value.isNumberlike());

    ASSERT_TRUE(!dbor::Value().isNumberlike());
    ASSERT_TRUE(!ValueBuilder{0xFB}.value.isNumberlike());
    ASSERT_TRUE(!ValueBuilder{0xFF}.value.isNumberlike());
}


static void testIsNumber() {
    // IntegerValue
    ASSERT_TRUE(ValueBuilder{0x00}.value.isNumber());
    ASSERT_TRUE((ValueBuilder{0x3F, 0x00}).value.isNumber());

    // BinaryRationalValue
    ASSERT_TRUE(ValueBuilder{0xC8}.value.isNumber());
    ASSERT_TRUE((ValueBuilder{0xCF, 0xFF}).value.isNumber());

    // DecimalRationalValue
    ASSERT_TRUE(ValueBuilder{0xD0}.value.isNumber());
    ASSERT_TRUE((ValueBuilder{0xEF, 0x00}).value.isNumber());

    ASSERT_TRUE(!dbor::Value().isNumber());
    ASSERT_TRUE(!ValueBuilder{0xFC}.value.isNumber());
    ASSERT_TRUE(!ValueBuilder{0xFD}.value.isNumber());
    ASSERT_TRUE(!ValueBuilder{0xFE}.value.isNumber());
    ASSERT_TRUE(!ValueBuilder{0xFF}.value.isNumber());
}


static void testIsString() {
    ASSERT_TRUE(ValueBuilder{0x40}.value.isString());
    ASSERT_TRUE((ValueBuilder{0x7F, 0x30}).value.isString());

    ASSERT_TRUE(!dbor::Value().isString());
    ASSERT_TRUE(!ValueBuilder{0x3F}.value.isString());
    ASSERT_TRUE(!ValueBuilder{0x80}.value.isString());
}


static void testIsContainer() {
    ASSERT_TRUE(ValueBuilder{0x80}.value.isContainer());
    ASSERT_TRUE((ValueBuilder{0xC7, 0xFF}).value.isContainer());

    ASSERT_TRUE(!dbor::Value().isString());
    ASSERT_TRUE(!ValueBuilder{0x7F}.value.isContainer());
    ASSERT_TRUE(!ValueBuilder{0xC8}.value.isContainer());
}


static void testIsXXXAreMutallyExclusive() {
    std::uint8_t b = 0;
    for (;;) {
        const std::uint8_t buffer[] = {b};
        dbor::Value value(buffer, sizeof(buffer));

        std::uint_least8_t n = 0;
        if (value.isNone())
            n++;
        if (value.isNumberlike())
            n++;
        if (value.isNumber())
            n++;
        if (value.isString())
            n++;
        if (value.isContainer())
            n++;

        ASSERT_TRUE(n <= 1);
        if (!(b >= 0xF0 && b < 0xFC))  // reserved?
            ASSERT_EQUAL(1, n);

        if (b++ == 0xFF)
            break;
    }
}


void testValue() {
    testDefaultConstructedIsEmpty();
    testIsEmptyWithoutBuffer();
    testSizeOfIncompleteIsCapacity();
    testSizeOfMultipleIsSizeOfFirst();

    testGetIntegerUint8();
    testGetIntegerUint16();
    testGetIntegerUint32();
    testGetIntegerUint64();

    testGetIntegerInt8();
    testGetIntegerInt16();
    testGetIntegerInt32();
    testGetIntegerInt64();

    testGetByteString();
    testGetUtf8String();

    testIsNone();
    testIsNumberlike();
    testIsNumber();
    testIsString();
    testIsContainer();
    testIsXXXAreMutallyExclusive();
}
