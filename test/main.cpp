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


static void testEncodingEncodeNaturalTokenData16() {
    {
        std::uint16_t value = 0;
        std::uint8_t buffer[] = { 7 };
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer)));
        ASSERT_EQUAL(7, buffer[0]);
    }

    {
        std::uint16_t value = 1;
        std::uint8_t buffer[] = { 7, 7 };
        ASSERT_EQUAL(1, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
        ASSERT_EQUAL(0, buffer[0]);
        ASSERT_EQUAL(7, buffer[1]);
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 2u));
    }

    {
        std::uint16_t value = 0x1234ul;
        std::uint8_t buffer[] = { 7, 7, 7 };
        ASSERT_EQUAL(2, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
        ASSERT_EQUAL(0x33, buffer[0]);
        ASSERT_EQUAL(0x11, buffer[1]);
        ASSERT_EQUAL(7, buffer[2]);
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 2u));
    }
}


static void testEncodingEncodeNaturalTokenData32() {
    {
        std::uint32_t value = 0;
        std::uint8_t buffer[] = { 7 };
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer)));
        ASSERT_EQUAL(7, buffer[0]);
    }

    {
        std::uint32_t value = 1;
        std::uint8_t buffer[] = { 7, 7 };
        ASSERT_EQUAL(1, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
        ASSERT_EQUAL(0, buffer[0]);
        ASSERT_EQUAL(7, buffer[1]);
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 2u));
    }

    {
        std::uint32_t value = 0x12345678ul;
        std::uint8_t buffer[] = { 7, 7, 7, 7, 7 };
        ASSERT_EQUAL(4, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
        ASSERT_EQUAL(0x77, buffer[0]);
        ASSERT_EQUAL(0x55, buffer[1]);
        ASSERT_EQUAL(0x33, buffer[2]);
        ASSERT_EQUAL(0x11, buffer[3]);
        ASSERT_EQUAL(7, buffer[4]);
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 2u));
    }
}


static void testEncodingEncodeNaturalTokenData64() {
    {
        std::uint64_t value = 0;
        std::uint8_t buffer[] = { 7 };
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer)));
        ASSERT_EQUAL(7, buffer[0]);
    }

    {
        std::uint64_t value = 1;
        std::uint8_t buffer[] = { 7, };
        ASSERT_EQUAL(1, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, buffer[0]);
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
    }

    {
        std::uint64_t value = 0x12345678ul;
        std::uint8_t buffer[] = { 7, 7, 7, 7, 7 };
        ASSERT_EQUAL(4, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
        ASSERT_EQUAL(0x77, buffer[0]);
        ASSERT_EQUAL(0x55, buffer[1]);
        ASSERT_EQUAL(0x33, buffer[2]);
        ASSERT_EQUAL(0x11, buffer[3]);
        ASSERT_EQUAL(7, buffer[4]);
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 2u));

        value = 0x100000000ul;
        ASSERT_EQUAL(4, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
        ASSERT_EQUAL(0xFF, buffer[0]);
        ASSERT_EQUAL(0xFE, buffer[1]);
        ASSERT_EQUAL(0xFE, buffer[2]);
        ASSERT_EQUAL(0xFE, buffer[3]);
        ASSERT_EQUAL(7, buffer[4]);
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 2u));

        value = 0x101010100ul;
        ASSERT_EQUAL(4, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
        ASSERT_EQUAL(0xFF, buffer[0]);
        ASSERT_EQUAL(0xFF, buffer[1]);
        ASSERT_EQUAL(0xFF, buffer[2]);
        ASSERT_EQUAL(0xFF, buffer[3]);
        ASSERT_EQUAL(7, buffer[4]);
    }
    {
        std::uint64_t value = 0x101010101ul;
        std::uint8_t buffer[] = { 7, 7, 7, 7, 7, 7 };

        ASSERT_EQUAL(5, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
        ASSERT_EQUAL(0x00, buffer[0]);
        ASSERT_EQUAL(0x00, buffer[1]);
        ASSERT_EQUAL(0x00, buffer[2]);
        ASSERT_EQUAL(0x00, buffer[3]);
        ASSERT_EQUAL(0x00, buffer[4]);
        ASSERT_EQUAL(7, buffer[5]);
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 2u));
    }

    {
        std::uint64_t value = 0x1234567887654321ull;
        std::uint8_t buffer[] = { 7, 7, 7, 7, 7, 7, 7, 7, 7 };
        ASSERT_EQUAL(8, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
        ASSERT_EQUAL(0x20, buffer[0]);
        ASSERT_EQUAL(0x42, buffer[1]);
        ASSERT_EQUAL(0x64, buffer[2]);
        ASSERT_EQUAL(0x86, buffer[3]);
        ASSERT_EQUAL(0x77, buffer[4]);
        ASSERT_EQUAL(0x55, buffer[5]);
        ASSERT_EQUAL(0x33, buffer[6]);
        ASSERT_EQUAL(0x11, buffer[7]);
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 2u));

        value = UINT64_MAX;
        ASSERT_EQUAL(8, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
        ASSERT_EQUAL(0xFE, buffer[0]);
        ASSERT_EQUAL(0xFE, buffer[1]);
        ASSERT_EQUAL(0xFE, buffer[2]);
        ASSERT_EQUAL(0xFE, buffer[3]);
        ASSERT_EQUAL(0xFE, buffer[4]);
        ASSERT_EQUAL(0xFE, buffer[5]);
        ASSERT_EQUAL(0xFE, buffer[6]);
        ASSERT_EQUAL(0xFE, buffer[7]);
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 2u));
    }

}


static void testEncoding() {
    testEncodingSizeInfoFromFirstByte();

    testEncodingDecodeNaturalTokenData16();
    testEncodingDecodeNaturalTokenData32();
    testEncodingDecodeNaturalTokenData64();

    testEncodingEncodeNaturalTokenData16();
    testEncodingEncodeNaturalTokenData32();
    testEncodingEncodeNaturalTokenData64();
}


int main() {
    testEncoding();
    return 0;
}
