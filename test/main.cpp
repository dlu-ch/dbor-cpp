// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "Dbor/Encoding.hpp"
#include "Test.hpp"


static void testEncodingSizeInfoFromFirstByte() {
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



static void testEncodingDecodeNaturalTokenData16() {
    {
        std::uint16_t value = 7;
        ASSERT_EQUAL(dbor::ErrorCode::OUT_OF_RANGE,
                     dbor::Encoding::decodeNaturalTokenData(value, nullptr, 0, 0));
        ASSERT_EQUAL(0, value);
    }

    {
        std::uint16_t value = 7;
        uint8_t buffer[] = { 0xFE, 0xFE, 0xFE };
        ASSERT_EQUAL(dbor::ErrorCode::OUT_OF_RANGE,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 0));
        ASSERT_EQUAL(0, value);
    }

    {
        std::uint16_t value = 7;
        uint8_t buffer[] = { 0x12 };
        ASSERT_EQUAL(dbor::ErrorCode::OK,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 23));
        ASSERT_EQUAL(0x13 + 23, value);
    }

    {
        std::uint16_t value = 7;
        uint8_t buffer[] = { 0xFE, 0xFE };
        ASSERT_EQUAL(dbor::ErrorCode::OK,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 0));
        ASSERT_EQUAL(UINT16_MAX, value);

        ASSERT_EQUAL(dbor::ErrorCode::OUT_OF_RANGE,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 1));
        ASSERT_EQUAL(0, value);
    }
}


static void testEncodingDecodeNaturalTokenData32() {
    {
        std::uint32_t value = 7;
        ASSERT_EQUAL(dbor::ErrorCode::OUT_OF_RANGE,
                     dbor::Encoding::decodeNaturalTokenData(value, nullptr, 0, 0));
        ASSERT_EQUAL(0, value);
    }

    {
        std::uint32_t value = 7;
        uint8_t buffer[] = { 0xFE, 0xFE, 0xFE, 0xFE, 0xFE };
        ASSERT_EQUAL(dbor::ErrorCode::OUT_OF_RANGE,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 0));
        ASSERT_EQUAL(0, value);
    }

    {
        std::uint32_t value = 7;
        uint8_t buffer[] = { 0x12, 0x23, 0x34 };
        ASSERT_EQUAL(dbor::ErrorCode::OK,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 23));
        ASSERT_EQUAL(0x352413 + 23, value);
    }

    {
        std::uint32_t value = 7;
        uint8_t buffer[] = { 0xFE, 0xFE, 0xFE, 0xFE };
        ASSERT_EQUAL(dbor::ErrorCode::OK,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 0));
        ASSERT_EQUAL(UINT32_MAX, value);

        ASSERT_EQUAL(dbor::ErrorCode::OUT_OF_RANGE,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 1));
        ASSERT_EQUAL(0, value);
    }
}


static void testEncodingDecodeNaturalTokenData64() {
    {
        std::uint64_t value = 7;
        ASSERT_EQUAL(dbor::ErrorCode::OUT_OF_RANGE,
                     dbor::Encoding::decodeNaturalTokenData(value, nullptr, 0, 0));
        ASSERT_EQUAL(0, value);
    }

    {
        std::uint64_t value = 7;
        uint8_t buffer[] = { 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE };
        ASSERT_EQUAL(dbor::ErrorCode::OUT_OF_RANGE,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 0));
        ASSERT_EQUAL(0, value);
    }

    {
        std::uint64_t value = 7;
        uint8_t buffer[] = { 0x12, 0x23, 0x34 };
        ASSERT_EQUAL(dbor::ErrorCode::OK,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 23));
        ASSERT_EQUAL(0x352413ull + 23, value);
    }

    {
        std::uint64_t value = 7;
        uint8_t buffer[] = { 0x12, 0x23, 0x34, 0x56, 0x78 };
        ASSERT_EQUAL(dbor::ErrorCode::OK,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 23));
        ASSERT_EQUAL(0x7957352413ull + 23, value);
    }

    {
        std::uint64_t value = 7;
        uint8_t buffer[] = { 0xFE, 0xFE, 0xFE, 0xFE };

        ASSERT_EQUAL(dbor::ErrorCode::OK,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 1));
        ASSERT_EQUAL(0x100000000ull, value);

        ASSERT_EQUAL(dbor::ErrorCode::OK,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 8));
        ASSERT_EQUAL(0x100000007ull, value);
    }
}


static void testEncoding() {
    testEncodingSizeInfoFromFirstByte();
    testEncodingDecodeNaturalTokenData16();
    testEncodingDecodeNaturalTokenData32();
    testEncodingDecodeNaturalTokenData64();
}


int main() {
    testEncoding();
    return 0;
}
