// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "Dbor/Encoding.hpp"
#include "Test.hpp"


static void testEncodingSizeOfTokenFromFirstByte() {
    // IntegerValue
    ASSERT_EQUAL(1, dbor::Encoding::sizeOfTokenFromFirstByte(0x00));
    ASSERT_EQUAL(1, dbor::Encoding::sizeOfTokenFromFirstByte(0x17));
    ASSERT_EQUAL(2, dbor::Encoding::sizeOfTokenFromFirstByte(0x18));
    ASSERT_EQUAL(9, dbor::Encoding::sizeOfTokenFromFirstByte(0x1F));
    ASSERT_EQUAL(9, dbor::Encoding::sizeOfTokenFromFirstByte(0x3F));

    // ByteStringValue
    ASSERT_EQUAL(1, dbor::Encoding::sizeOfTokenFromFirstByte(0x40));
    ASSERT_EQUAL(1, dbor::Encoding::sizeOfTokenFromFirstByte(0x57));
    ASSERT_EQUAL(2, dbor::Encoding::sizeOfTokenFromFirstByte(0x58));

    // DictionaryValue
    ASSERT_EQUAL(1, dbor::Encoding::sizeOfTokenFromFirstByte(0xA0));
    ASSERT_EQUAL(9, dbor::Encoding::sizeOfTokenFromFirstByte(0xBF));

    // AllocatedValue
    ASSERT_EQUAL(2, dbor::Encoding::sizeOfTokenFromFirstByte(0xC0));
    ASSERT_EQUAL(9, dbor::Encoding::sizeOfTokenFromFirstByte(0xC7));

    // BinaryRationalValue
    ASSERT_EQUAL(2, dbor::Encoding::sizeOfTokenFromFirstByte(0xC8));
    ASSERT_EQUAL(9, dbor::Encoding::sizeOfTokenFromFirstByte(0xCF));

    // DecimalRationalValue(..., e) with |e| > 8
    ASSERT_EQUAL(2, dbor::Encoding::sizeOfTokenFromFirstByte(0xD0));
    ASSERT_EQUAL(9, dbor::Encoding::sizeOfTokenFromFirstByte(0xDF));

    // DecimalRationalValue(..., e) with |e| <= 8
    ASSERT_EQUAL(1, dbor::Encoding::sizeOfTokenFromFirstByte(0xE0));
    ASSERT_EQUAL(1, dbor::Encoding::sizeOfTokenFromFirstByte(0xEF));

    // MinimalToken
    ASSERT_EQUAL(1, dbor::Encoding::sizeOfTokenFromFirstByte(0xFC));
    ASSERT_EQUAL(1, dbor::Encoding::sizeOfTokenFromFirstByte(0xFD));
    ASSERT_EQUAL(1, dbor::Encoding::sizeOfTokenFromFirstByte(0xFE));
    ASSERT_EQUAL(1, dbor::Encoding::sizeOfTokenFromFirstByte(0xFF));
}


static void testEncodingSizeOfValueIn() {
    ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(nullptr, 0u));

    // IntegerValue
    {
        uint8_t buffer[] = { 0x00 };
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = { 0x1F };
        ASSERT_EQUAL(9, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = { 0x37 };
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = { 0x38 };
        ASSERT_EQUAL(2, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // ByteStringValue
    {
        uint8_t buffer[] = { 0x40 };
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = { 0x57 };
        ASSERT_EQUAL(1 + 23, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // Utf8StringValue
    {
        uint8_t buffer[] = { 0x77 };
        ASSERT_EQUAL(1 + 23, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = { 0x78, 0x00 };
        ASSERT_EQUAL(2 + 24, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 1));
    }
    {
        uint8_t buffer[] = { 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // SequenceValue
    {
        uint8_t buffer[] = { 0x80 };
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = { 0x97 };
        ASSERT_EQUAL(1 + 23, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = { 0x98, 0xFF };
        ASSERT_EQUAL(2 + 24 + 255, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 1));
    }

    // DictionaryValue
    {
        uint8_t buffer[] = { 0xA0 };
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = { 0xB7 };
        ASSERT_EQUAL(1 + 23, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = { 0xB9, 0x00, 0x00 };
        ASSERT_EQUAL(3 + 24 + 256, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 1));
    }

    // AllocatedValue
    {
        uint8_t buffer[] = { 0xC0, 0x00 };
        ASSERT_EQUAL(2 + 1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 1));
    }
    {
        uint8_t buffer[] = { 0xC0, 0xFF };
        ASSERT_EQUAL(2 + 256, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // BinaryRationalValue
    {
        uint8_t buffer[] = { 0xC8 };
        ASSERT_EQUAL(2, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = { 0xCF };
        ASSERT_EQUAL(9, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // DecimalRationalValue 1101xyyy
    {
        uint8_t buffer[] = { 0xD0, 0xFF, 0x00 };
        ASSERT_EQUAL(3, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 1));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 2));
    }
    {
        uint8_t buffer[] = { 0xD1, 0xFF, 0x00, 0x38 };
        ASSERT_EQUAL(3 + 2, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 1));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 2));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 3));
    }
    {
        uint8_t buffer[] = { 0xD1, 0xFF, 0x00, 0xFF };  // ill-formed
        ASSERT_EQUAL(3, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = { 0xE0, 0x3F };
        ASSERT_EQUAL(1 + 9, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 1));
    }
    {
        uint8_t buffer[] = { 0xEF, 0xFF };  // ill-formed
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // NumberlikeValue
    {
        uint8_t buffer[] = { 0xFC };
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = { 0xFD };
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = { 0xFE };
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // NoneValue
    {
        uint8_t buffer[] = { 0xFF };
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // reserved
    {
        uint8_t buffer[] = { 0xF0 };
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
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
    testEncodingSizeOfTokenFromFirstByte();
    testEncodingSizeOfValueIn();

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
