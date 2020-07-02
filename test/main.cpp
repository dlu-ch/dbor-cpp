// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "Dbor/Encoding.hpp"
#include "Test.hpp"


static void test_encoding() {
    // IntegerValue
    ASSERT_EQUAL(1, dbor::Encoding::sizeInfoFromFirstByte(0x00));
    ASSERT_EQUAL(1, dbor::Encoding::sizeInfoFromFirstByte(0x17));
    ASSERT_EQUAL(2, dbor::Encoding::sizeInfoFromFirstByte(0x18));
    ASSERT_EQUAL(9, dbor::Encoding::sizeInfoFromFirstByte(0x1F));
    ASSERT_EQUAL(9, dbor::Encoding::sizeInfoFromFirstByte(0x3F));

    // ByteStringValue
    ASSERT_EQUAL(1 + 0, dbor::Encoding::sizeInfoFromFirstByte(0x40));
    ASSERT_EQUAL(1 + 23, dbor::Encoding::sizeInfoFromFirstByte(0x57));
    ASSERT_EQUAL((2 + 23) | 0x40, dbor::Encoding::sizeInfoFromFirstByte(0x58));

    // DictionaryValue
    ASSERT_EQUAL(1 + 0, dbor::Encoding::sizeInfoFromFirstByte(0xA0));
    ASSERT_EQUAL((9 + 23) | 0x40, dbor::Encoding::sizeInfoFromFirstByte(0xBF));

    // AllocatedValue
    ASSERT_EQUAL(2 | 0x40, dbor::Encoding::sizeInfoFromFirstByte(0xC0));
    ASSERT_EQUAL(9 | 0x40, dbor::Encoding::sizeInfoFromFirstByte(0xC7));

    // BinaryRationalValue
    ASSERT_EQUAL(2, dbor::Encoding::sizeInfoFromFirstByte(0xC8));
    ASSERT_EQUAL(9, dbor::Encoding::sizeInfoFromFirstByte(0xCF));

    // DecimalRationalValue(..., e) with |e| > 8
    ASSERT_EQUAL(2 | 0x80, dbor::Encoding::sizeInfoFromFirstByte(0xD0));
    ASSERT_EQUAL(9 | 0x80, dbor::Encoding::sizeInfoFromFirstByte(0xDF));

    // DecimalRationalValue(..., e) with |e| <= 8
    ASSERT_EQUAL(1 | 0x80, dbor::Encoding::sizeInfoFromFirstByte(0xE0));
    ASSERT_EQUAL(1 | 0x80, dbor::Encoding::sizeInfoFromFirstByte(0xEF));

    // MinimalToken
    ASSERT_EQUAL(1, dbor::Encoding::sizeInfoFromFirstByte(0xFC));
    ASSERT_EQUAL(1, dbor::Encoding::sizeInfoFromFirstByte(0xFD));
    ASSERT_EQUAL(1, dbor::Encoding::sizeInfoFromFirstByte(0xFE));
    ASSERT_EQUAL(1, dbor::Encoding::sizeInfoFromFirstByte(0xFF));
}

int main() {
    test_encoding();
    return 0;
}
