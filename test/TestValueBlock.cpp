// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "TestValueBlock.hpp"
#include "Test.hpp"
#include "Dbor/ValueBlock.hpp"


static void testValueBlockPreservesBufferAndCapacity() {
    const void *p = "x";
    ASSERT_EQUAL(p, dbor::ValueBlock(p, 4).buffer());
    ASSERT_EQUAL(4, dbor::ValueBlock(p, 4).capacity());
    ASSERT_EQUAL(p, dbor::ValueBlock(p, 0).buffer());
    ASSERT_EQUAL(4, dbor::ValueBlock(nullptr, 4).capacity());
}


static void testIsEmptyWithoutBuffer() {
    dbor::ValueBlock values(nullptr, 2);
    ASSERT_TRUE(values.begin() == values.end());
    ASSERT_TRUE(values.empty());
}


static void testIsEmptyWithEmptyBuffer() {
    dbor::ValueBlock values("", 0);
    ASSERT_TRUE(values.begin() == values.end());
    ASSERT_TRUE(values.empty());
}


static void testAllEmptyIteratorsAreEqual() {
    ASSERT_TRUE(dbor::ValueBlock::Iterator() == dbor::ValueBlock::Iterator());
    ASSERT_TRUE(!(dbor::ValueBlock::Iterator() != dbor::ValueBlock::Iterator()));

    ASSERT_TRUE(dbor::ValueBlock::Iterator("x", 0) == dbor::ValueBlock::Iterator());
    ASSERT_TRUE(dbor::ValueBlock::Iterator("x", 0) == dbor::ValueBlock::Iterator("y", 0));
}


static void testIsAtEndForDefaultConstructed() {
    dbor::ValueBlock::Iterator iter;

    ASSERT_EQUAL(0, iter.remainingSize());
    ASSERT_EQUAL(nullptr, (*iter).buffer());
    ASSERT_EQUAL(nullptr, iter->buffer());
    ASSERT_TRUE(iter.isAtEnd());
}


static void testIsAtEndWithoutBuffer() {
    dbor::ValueBlock::Iterator iter(nullptr, 3);

    ASSERT_EQUAL(0, iter.remainingSize());
    ASSERT_EQUAL(nullptr, (*iter).buffer());
    ASSERT_TRUE(iter.isAtEnd());
}


static void testIsAtEndWithEmptyBuffer() {
    dbor::ValueBlock::Iterator iter("", 0);

    ASSERT_EQUAL(0, iter.remainingSize());
    ASSERT_EQUAL(nullptr, (*iter).buffer());
    ASSERT_EQUAL(nullptr, iter->buffer());
    ASSERT_TRUE(iter.isAtEnd());
}


static void testIsNonemptyWithIncomplete() {
    const std::uint8_t buffer[] = { 0x1F, 0x00 };
    dbor::ValueBlock values(buffer, sizeof(buffer));
    dbor::ValueBlock::Iterator iter(buffer, sizeof(buffer));

    ASSERT_TRUE(!values.empty());
    ASSERT_TRUE(values.begin() != values.end());
    ASSERT_EQUAL(values.begin(), iter);

    ASSERT_EQUAL(0, iter.remainingSize());
    ASSERT_TRUE(!iter.isAtEnd());
    ASSERT_EQUAL(buffer, (*iter).buffer());
    ASSERT_EQUAL(buffer, iter->buffer());
    ASSERT_EQUAL(sizeof(buffer), iter->size());
}


static void testIteratesOverAllIfWellformed() {
    const std::uint8_t buffer[] = { 0xFF, 0x18, 0x00, 0xA0 };
    dbor::ValueBlock values(buffer, sizeof(buffer));
    dbor::ValueBlock::Iterator iter(buffer, sizeof(buffer));

    ASSERT_TRUE(!values.empty());
    ASSERT_TRUE(values.begin() != values.end());
    ASSERT_EQUAL(values.begin(), iter);

    ASSERT_EQUAL(buffer, iter->buffer());
    ASSERT_EQUAL(1, iter->size());
    ASSERT_TRUE(!iter.isAtEnd());

    ++iter;

    ASSERT_EQUAL(buffer + 1, iter->buffer());
    ASSERT_EQUAL(2, iter->size());
    ASSERT_TRUE(!iter.isAtEnd());

    ++iter;

    ASSERT_EQUAL(buffer + 3, iter->buffer());
    ASSERT_EQUAL(1, iter->size());
    ASSERT_TRUE(!iter.isAtEnd());

    ++iter;

    ASSERT_EQUAL(nullptr, iter->buffer());
    ASSERT_EQUAL(0, iter->size());
    ASSERT_TRUE(iter.isAtEnd());
    ASSERT_EQUAL(values.end(), iter);

    ++iter;

    ASSERT_EQUAL(nullptr, iter->buffer());
    ASSERT_EQUAL(0, iter->size());
    ASSERT_TRUE(iter.isAtEnd());
    ASSERT_EQUAL(values.end(), iter);
}


static void testIteratesOverAllIfLastIsIncomplete() {
    const std::uint8_t buffer[] = { 0x18, 0x00, 0xA0, 0x1F, 0x00 };
    dbor::ValueBlock values(buffer, sizeof(buffer));
    dbor::ValueBlock::Iterator iter(buffer, sizeof(buffer));

    ASSERT_TRUE(!values.empty());
    ASSERT_TRUE(values.begin() != values.end());
    ASSERT_EQUAL(values.begin(), iter);

    ASSERT_EQUAL(buffer, iter->buffer());
    ASSERT_EQUAL(2, iter->size());
    ASSERT_TRUE(!iter.isAtEnd());

    ++iter;

    ASSERT_EQUAL(buffer + 2, iter->buffer());
    ASSERT_EQUAL(1, iter->size());
    ASSERT_TRUE(!iter.isAtEnd());

    ++iter;

    ASSERT_EQUAL(buffer + 3, iter->buffer());
    ASSERT_EQUAL(2, iter->size());
    ASSERT_TRUE(!iter.isAtEnd());

    ++iter;

    ASSERT_EQUAL(nullptr, iter->buffer());
    ASSERT_EQUAL(0, iter->size());
    ASSERT_TRUE(iter.isAtEnd());
    ASSERT_EQUAL(values.end(), iter);

    ++iter;

    ASSERT_EQUAL(nullptr, iter->buffer());
    ASSERT_EQUAL(0, iter->size());
    ASSERT_TRUE(iter.isAtEnd());
    ASSERT_EQUAL(values.end(), iter);
}


static void testIteratesOverIllformedDecimalRational() {
    const std::uint8_t buffer[] = { 0xE0, 0xE1, 0xE2 };
    dbor::ValueBlock values(buffer, sizeof(buffer));
    dbor::ValueBlock::Iterator iter(buffer, sizeof(buffer));

    ASSERT_TRUE(!values.empty());
    ASSERT_TRUE(values.begin() != values.end());
    ASSERT_EQUAL(values.begin(), iter);

    ASSERT_EQUAL(buffer, iter->buffer());
    ASSERT_EQUAL(1, iter->size());
    ASSERT_TRUE(!iter.isAtEnd());

    ++iter;

    ASSERT_EQUAL(buffer + 1, iter->buffer());
    ASSERT_EQUAL(1, iter->size());
    ASSERT_TRUE(!iter.isAtEnd());

    ++iter;

    ASSERT_EQUAL(buffer + 2, iter->buffer());
    ASSERT_EQUAL(1, iter->size());
    ASSERT_TRUE(!iter.isAtEnd());

    ++iter;

    ASSERT_EQUAL(nullptr, iter->buffer());
    ASSERT_EQUAL(0, iter->size());
    ASSERT_TRUE(iter.isAtEnd());
    ASSERT_EQUAL(values.end(), iter);
}


static void testCanBeUsedInRangeIteration() {
    const std::uint8_t buffer[] = { 0xFF, 12 };
    std::size_t n = 0;
    for (const dbor::Value &v: dbor::ValueBlock(buffer, sizeof(buffer))) {
        ASSERT_TRUE(v.buffer() >= buffer);
        ASSERT_EQUAL(1, v.size());
        n++;
    }
    ASSERT_EQUAL(2, n);
}


void testValueIterator() {
    testValueBlockPreservesBufferAndCapacity();
    testIsEmptyWithoutBuffer();
    testIsEmptyWithEmptyBuffer();

    testAllEmptyIteratorsAreEqual();
    testIsAtEndForDefaultConstructed();
    testIsAtEndWithoutBuffer();
    testIsAtEndWithEmptyBuffer();
    testIsNonemptyWithIncomplete();
    testIteratesOverAllIfWellformed();
    testIteratesOverAllIfLastIsIncomplete();
    testIteratesOverIllformedDecimalRational();
    testCanBeUsedInRangeIteration();
}
