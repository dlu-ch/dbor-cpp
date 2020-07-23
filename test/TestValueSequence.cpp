// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "TestValueSequence.hpp"
#include "Test.hpp"
#include "Dbor/ValueSequence.hpp"


static void testValueSequencePreservesBufferAndCapacity() {
    const void *p = "x";
    ASSERT_EQUAL(p, dbor::ValueSequence(p, 4).buffer());
    ASSERT_EQUAL(4, dbor::ValueSequence(p, 4).capacity());
    ASSERT_EQUAL(p, dbor::ValueSequence(p, 0).buffer());
    ASSERT_EQUAL(4, dbor::ValueSequence(nullptr, 4).capacity());
}


static void testIsEmptyWithoutBuffer() {
    dbor::ValueSequence values(nullptr, 2);
    ASSERT_TRUE(values.begin() == values.end());
    ASSERT_TRUE(values.empty());
}


static void testIsEmptyWithEmptyBuffer() {
    dbor::ValueSequence values("", 0);
    ASSERT_TRUE(values.begin() == values.end());
    ASSERT_TRUE(values.empty());
}


static void testAllEmptyIteratorsAreEqual() {
    ASSERT_TRUE(dbor::ValueSequence::Iterator() == dbor::ValueSequence::Iterator());
    ASSERT_TRUE(!(dbor::ValueSequence::Iterator() != dbor::ValueSequence::Iterator()));

    ASSERT_TRUE(dbor::ValueSequence::Iterator("x", 0) == dbor::ValueSequence::Iterator());
    ASSERT_TRUE(dbor::ValueSequence::Iterator("x", 0) == dbor::ValueSequence::Iterator("y", 0));
}


static void testIsAtEndForDefaultConstructed() {
    dbor::ValueSequence::Iterator iter;

    ASSERT_EQUAL(0, iter.remainingSize());
    ASSERT_EQUAL(nullptr, (*iter).buffer());
    ASSERT_EQUAL(nullptr, iter->buffer());
    ASSERT_TRUE(iter.isAtEnd());
}


static void testIsAtEndWithoutBuffer() {
    dbor::ValueSequence::Iterator iter(nullptr, 3);

    ASSERT_EQUAL(0, iter.remainingSize());
    ASSERT_EQUAL(nullptr, (*iter).buffer());
    ASSERT_TRUE(iter.isAtEnd());
}


static void testIsAtEndWithEmptyBuffer() {
    dbor::ValueSequence::Iterator iter("", 0);

    ASSERT_EQUAL(0, iter.remainingSize());
    ASSERT_EQUAL(nullptr, (*iter).buffer());
    ASSERT_EQUAL(nullptr, iter->buffer());
    ASSERT_TRUE(iter.isAtEnd());
}


static void testIsNonemptyWithIncomplete() {
    const std::uint8_t buffer[] = {0x1F, 0x00};
    dbor::ValueSequence values(buffer, sizeof(buffer));
    dbor::ValueSequence::Iterator iter(buffer, sizeof(buffer));

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
    const std::uint8_t buffer[] = {0xFF, 0x18, 0x00, 0xA0};
    dbor::ValueSequence values(buffer, sizeof(buffer));
    dbor::ValueSequence::Iterator iter(buffer, sizeof(buffer));

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
    const std::uint8_t buffer[] = {0x18, 0x00, 0xA0, 0x1F, 0x00};
    dbor::ValueSequence values(buffer, sizeof(buffer));
    dbor::ValueSequence::Iterator iter(buffer, sizeof(buffer));

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
    const std::uint8_t buffer[] = {0xE0, 0xE1, 0xE2};
    dbor::ValueSequence values(buffer, sizeof(buffer));
    dbor::ValueSequence::Iterator iter(buffer, sizeof(buffer));

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
    const std::uint8_t buffer[] = {0xFF, 12};
    std::size_t n = 0;
    for (const dbor::Value &v: dbor::ValueSequence(buffer, sizeof(buffer))) {
        ASSERT_TRUE(v.buffer() >= buffer);
        ASSERT_EQUAL(1, v.size());
        n++;
    }
    ASSERT_EQUAL(2, n);
}


static void testChainedDecoding() {
    const std::uint8_t buffer[] = {0xFF, 12, 0xFE};
    dbor::ValueSequence values(buffer, sizeof(buffer));
    auto iter = values.begin();

    ASSERT_EQUAL(buffer, iter->buffer());

    std::uint8_t a, b, c;
    dbor::ResultCodeSet results =
               iter->get(a)
        << (++iter)->get(b)
        << (++iter)->get(c);

    ASSERT_EQUAL(0, a);
    ASSERT_EQUAL(12, b);
    ASSERT_EQUAL(0xFF, c);

    ASSERT_EQUAL(dbor::ResultCode::NO_OBJECT << dbor::ResultCode::APPROX_EXTREME, results);
    ASSERT_TRUE(!isOk(results));
    ASSERT_TRUE(isOkExcept(results,
                           dbor::ResultCode::NO_OBJECT << dbor::ResultCode::APPROX_EXTREME));
}


void testValueSequence() {
    testValueSequencePreservesBufferAndCapacity();
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
    testChainedDecoding();
}
