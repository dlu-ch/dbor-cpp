// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "TestEncoding.hpp"
#include "Test.hpp"
#include "Dbor/Encoding.hpp"


static void testSizeOfTokenFromFirstByte() {
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


static void testSizeOfValueIn() {
    ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(nullptr, 0u));

    // IntegerValue
    {
        uint8_t buffer[] = {0x00};
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = {0x1F};
        ASSERT_EQUAL(9, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = {0x37};
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = {0x38};
        ASSERT_EQUAL(2, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // ByteStringValue
    {
        uint8_t buffer[] = {0x40};
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = {0x57};
        ASSERT_EQUAL(1 + 23, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // Utf8StringValue
    {
        uint8_t buffer[] = {0x77};
        ASSERT_EQUAL(1 + 23, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = {0x78, 0x00};
        ASSERT_EQUAL(2 + 24, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 1));
    }
    {
        uint8_t buffer[] = {0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // SequenceValue
    {
        uint8_t buffer[] = {0x80};
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = {0x97};
        ASSERT_EQUAL(1 + 23, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = {0x98, 0xFF};
        ASSERT_EQUAL(2 + 24 + 255, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 1));
    }

    // DictionaryValue
    {
        uint8_t buffer[] = {0xA0};
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = {0xB7};
        ASSERT_EQUAL(1 + 23, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = {0xB9, 0x00, 0x00};
        ASSERT_EQUAL(3 + 24 + 256, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 1));
    }

    // AllocatedValue
    {
        uint8_t buffer[] = {0xC0, 0x00};
        ASSERT_EQUAL(2 + 1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 1));
    }
    {
        uint8_t buffer[] = {0xC0, 0xFF};
        ASSERT_EQUAL(2 + 256, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // BinaryRationalValue
    {
        uint8_t buffer[] = {0xC8};
        ASSERT_EQUAL(2, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = {0xCF};
        ASSERT_EQUAL(9, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // DecimalRationalValue 1101xyyy
    {
        uint8_t buffer[] = {0xD0, 0xFF, 0x00};
        ASSERT_EQUAL(3, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 1));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 2));
    }
    {
        uint8_t buffer[] = {0xD1, 0xFF, 0x00, 0x38};
        ASSERT_EQUAL(3 + 2, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 1));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 2));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 3));
    }
    {
        uint8_t buffer[] = {0xD1, 0xFF, 0x00, 0xFF};  // ill-formed
        ASSERT_EQUAL(3, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = {0xE0, 0x3F};
        ASSERT_EQUAL(1 + 9, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer) - 1));
    }
    {
        uint8_t buffer[] = {0xEF, 0xFF};  // ill-formed
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // NumberlikeValue
    {
        uint8_t buffer[] = {0xFC};
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = {0xFD};
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
    {
        uint8_t buffer[] = {0xFE};
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // NoneValue
    {
        uint8_t buffer[] = {0xFF};
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }

    // reserved
    {
        uint8_t buffer[] = {0xF0};
        ASSERT_EQUAL(1, dbor::Encoding::sizeOfValueIn(buffer, sizeof(buffer)));
    }
}


static void testDecodeNaturalTokenData16() {
    {
        std::uint16_t value = 7;
        ASSERT_EQUAL(false,
                     dbor::Encoding::decodeNaturalTokenData(value, nullptr, 0, 0));
        ASSERT_EQUAL(0, value);
    }

    {
        std::uint16_t value = 7;
        uint8_t buffer[] = {0xFE, 0xFE, 0xFE};
        ASSERT_EQUAL(false,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 0));
        ASSERT_EQUAL(0, value);
    }

    {
        std::uint16_t value = 7;
        uint8_t buffer[] = {0x12};
        ASSERT_EQUAL(true,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 23));
        ASSERT_EQUAL(0x13 + 23, value);
    }

    {
        std::uint16_t value = 7;
        uint8_t buffer[] = {0xFE, 0xFE};
        ASSERT_EQUAL(true,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 0));
        ASSERT_EQUAL(UINT16_MAX, value);

        ASSERT_EQUAL(false,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 1));
        ASSERT_EQUAL(0, value);
    }
}


static void testDecodeNaturalTokenData32() {
    {
        std::uint32_t value = 7;
        ASSERT_EQUAL(false,
                     dbor::Encoding::decodeNaturalTokenData(value, nullptr, 0, 0));
        ASSERT_EQUAL(0, value);
    }

    {
        std::uint32_t value = 7;
        uint8_t buffer[] = {0xFE, 0xFE, 0xFE, 0xFE, 0xFE};
        ASSERT_EQUAL(false,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 0));
        ASSERT_EQUAL(0, value);
    }

    {
        std::uint32_t value = 7;
        uint8_t buffer[] = {0x12, 0x23, 0x34};
        ASSERT_EQUAL(true,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 23));
        ASSERT_EQUAL(0x352413 + 23, value);
    }

    {
        std::uint32_t value = 7;
        uint8_t buffer[] = {0xFE, 0xFE, 0xFE, 0xFE};
        ASSERT_EQUAL(true,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 0));
        ASSERT_EQUAL(UINT32_MAX, value);

        ASSERT_EQUAL(false,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 1));
        ASSERT_EQUAL(0, value);
    }
}


static void testDecodeNaturalTokenData64() {
    {
        std::uint64_t value = 7;
        ASSERT_EQUAL(false,
                     dbor::Encoding::decodeNaturalTokenData(value, nullptr, 0, 0));
        ASSERT_EQUAL(0, value);
    }

    {
        std::uint64_t value = 7;
        uint8_t buffer[] = {0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE};
        ASSERT_EQUAL(false,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 0));
        ASSERT_EQUAL(0, value);
    }

    {
        std::uint64_t value = 7;
        uint8_t buffer[] = {0x12, 0x23, 0x34};
        ASSERT_EQUAL(true,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 23));
        ASSERT_EQUAL(0x352413ull + 23, value);
    }

    {
        std::uint64_t value = 7;
        uint8_t buffer[] = {0x12, 0x23, 0x34, 0x56, 0x78};
        ASSERT_EQUAL(true,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 23));
        ASSERT_EQUAL(0x7957352413ull + 23, value);
    }

    {
        std::uint64_t value = 7;
        uint8_t buffer[] = {0xFE, 0xFE, 0xFE, 0xFE};

        ASSERT_EQUAL(true,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 1));
        ASSERT_EQUAL(0x100000000ull, value);

        ASSERT_EQUAL(true,
                     dbor::Encoding::decodeNaturalTokenData(value, buffer, sizeof(buffer), 8));
        ASSERT_EQUAL(0x100000007ull, value);
    }
}


static void testEncodeNaturalTokenData16() {
    {
        std::uint16_t value = 0;
        std::uint8_t buffer[] = {7};
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer)));
        ASSERT_EQUAL(7, buffer[0]);
    }

    {
        std::uint16_t value = 1;
        std::uint8_t buffer[] = {7, 7};
        ASSERT_EQUAL(1, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
        ASSERT_EQUAL(0, buffer[0]);
        ASSERT_EQUAL(7, buffer[1]);
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 2u));
    }

    {
        std::uint16_t value = 0x1234ul;
        std::uint8_t buffer[] = {7, 7, 7};
        ASSERT_EQUAL(2, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
        ASSERT_EQUAL(0x33, buffer[0]);
        ASSERT_EQUAL(0x11, buffer[1]);
        ASSERT_EQUAL(7, buffer[2]);
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 2u));
    }
}


static void testEncodeNaturalTokenData32() {
    {
        std::uint32_t value = 0;
        std::uint8_t buffer[] = {7};
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer)));
        ASSERT_EQUAL(7, buffer[0]);
    }

    {
        std::uint32_t value = 1;
        std::uint8_t buffer[] = {7, 7};
        ASSERT_EQUAL(1, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
        ASSERT_EQUAL(0, buffer[0]);
        ASSERT_EQUAL(7, buffer[1]);
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 2u));
    }

    {
        std::uint32_t value = 0x12345678ul;
        std::uint8_t buffer[] = {7, 7, 7, 7, 7};
        ASSERT_EQUAL(4, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
        ASSERT_EQUAL(0x77, buffer[0]);
        ASSERT_EQUAL(0x55, buffer[1]);
        ASSERT_EQUAL(0x33, buffer[2]);
        ASSERT_EQUAL(0x11, buffer[3]);
        ASSERT_EQUAL(7, buffer[4]);
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 2u));
    }
}


static void testEncodeNaturalTokenData64() {
    {
        std::uint64_t value = 0;
        std::uint8_t buffer[] = {7};
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer)));
        ASSERT_EQUAL(7, buffer[0]);
    }

    {
        std::uint64_t value = 1;
        std::uint8_t buffer[] = {7,};
        ASSERT_EQUAL(1, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer)));
        ASSERT_EQUAL(0, buffer[0]);
        ASSERT_EQUAL(0, dbor::Encoding::encodeNaturalTokenData(value, buffer, sizeof(buffer) - 1u));
    }

    {
        std::uint64_t value = 0x12345678ul;
        std::uint8_t buffer[] = {7, 7, 7, 7, 7};
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
        std::uint8_t buffer[] = {7, 7, 7, 7, 7, 7};

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
        std::uint8_t buffer[] = {7, 7, 7, 7, 7, 7, 7, 7, 7};
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


static void testDecodeBinaryRationalTokenDataAs32b() {
    std::uint32_t v;

    const auto decode = dbor::Encoding::decodeBinaryRationalTokenDataAs32b;

    // k = 0
    // r = 3
    // p = 4
    // e = E - 2^(r-1) + 1 = E - 3

    // 2^-3                              sEEEMMMM
    v = decode(test::ByteBufferBuilder{0b00000000}.buffer, 0);
    ASSERT_EQUAL(0b00111110000000000000000000000000ul, v);
    //             sEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM,  E' = e + 127 = -3 + 127 = 0x7C

    // -(1 + 0b1111 / 2^4) * 2^4         sEEEMMMM
    v = decode(test::ByteBufferBuilder{0b11111111}.buffer, 0);
    ASSERT_EQUAL(0b11000001111110000000000000000000ul, v);
    //             sEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM,  E' = e + 127 = 4 + 127 = 0x83

    // k = 1
    // r = 5
    // p = 10
    // e = E - 2^(r-1) + 1 = E - 15

    // 2^-15                             MMMMMMMM    sEEEEEMM
    v = decode(test::ByteBufferBuilder{0b00000000, 0b00000000}.buffer, 1);
    ASSERT_EQUAL(0b00111000000000000000000000000000ul, v);
    //             sEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM,  E' = e + 127 = -15 + 127 = 0x70

    // -(1 + 0b1111111111 / 2^10) * 2^16
    //                                   MMMMMMMM    sEEEEEMM
    v = decode(test::ByteBufferBuilder{0b11111111, 0b11111111}.buffer, 1);
    ASSERT_EQUAL(0b11000111111111111110000000000000ul, v);
    //             sEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM,  E' = e + 127 = 16 + 127 = 0x8F

    // k = 2
    // r = 7
    // p = 16
    // e = E - 2^(r-1) + 1 = E - 63

    // 2^-63                             MMMMMMMM    MMMMMMMM    sEEEEEEE
    v = decode(test::ByteBufferBuilder{0b00000000, 0b00000000, 0b00000000}.buffer, 2);
    ASSERT_EQUAL(0b00100000000000000000000000000000ul, v);
    //             sEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM,  E' = e + 127 = -63 + 127 = 0x40

    // -(1 + 0b1111111111111111 / 2^16) * 2^64
    //                                   MMMMMMMM    MMMMMMMM    sEEEEEEE
    v = decode(test::ByteBufferBuilder{0b11111111, 0b11111111, 0b11111111}.buffer, 2);
    ASSERT_EQUAL(0b11011111111111111111111110000000ul, v);
    //             sEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM,  E' = e + 127 = 64 + 127 = 0xBF

    // k = 3
    // r = 8
    // p = 23
    // e = E - 2^(r-1) + 1 = E - 127

    // 2^-127                            MMMMMMMM    MMMMMMMM    EMMMMMMM    sEEEEEEE
    v = decode(test::ByteBufferBuilder{0b00000000, 0b00000000, 0b00000000, 0b00000000}.buffer, 3);
    ASSERT_EQUAL(0b00000000000000000000000000000000ul, v);
    //             sEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM,  E' = e + 127 = -127 + 127 = 0x00

    // -(1 + 0b11111111111111111111111 / 2^23) * 2^128
    //                                   MMMMMMMM    MMMMMMMM    EMMMMMMM    sEEEEEEE
    v = decode(test::ByteBufferBuilder{0b11111111, 0b11111111, 0b11111111, 0b11111111}.buffer, 3);
    ASSERT_EQUAL(0b11111111111111111111111111111111ul, v);
    //             sEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM,  E' = e + 127 = 128 + 127 = 0xFF
}


static void testDecodeBinaryRationalTokenDataAs64b() {
    std::uint64_t v;

    const auto decode = dbor::Encoding::decodeBinaryRationalTokenDataAs64b;

    // k = 4
    // r = 9
    // p = 30
    // e = E - 2^(r-1) + 1 = E - 255

    // 2^-255
    v = decode(test::ByteBufferBuilder{
    //    MMMMMMMM    MMMMMMMM    MMMMMMMM    EEMMMMMM
        0b00000000, 0b00000000, 0b00000000, 0b00000000,
        0b00000000
    //    sEEEEEEE
    }.buffer, 4);
    ASSERT_EQUAL(0b0011000000000000000000000000000000000000000000000000000000000000ull, v);
    //             sEEEEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    // E' = e + 1023 = -255 + 1023 = 0x300

    // -(1 + 0b111... / 2^30) * 2^256
    v = decode(test::ByteBufferBuilder{
    //    MMMMMMMM    MMMMMMMM    MMMMMMMM    EEMMMMMM
        0b11111111, 0b11111111, 0b11111111, 0b11111111,
        0b11111111
    //    sEEEEEEE
    }.buffer, 4);
    ASSERT_EQUAL(0b1100111111111111111111111111111111111111110000000000000000000000ull, v);
    //             sEEEEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    // E' = e + 1023 = 256 + 1023 = 0x4FF

    // k = 5
    // r = 10
    // p = 37
    // e = E - 2^(r-1) + 1 = E - 511

    // 2^-511
    v = decode(test::ByteBufferBuilder{
    //    MMMMMMMM    MMMMMMMM    MMMMMMMM    MMMMMMMM
        0b00000000, 0b00000000, 0b00000000, 0b00000000,
        0b00000000, 0b00000000
    //    EEEMMMMM    sEEEEEEE
    }.buffer, 5);
    ASSERT_EQUAL(0b0010000000000000000000000000000000000000000000000000000000000000ull, v);
    //             sEEEEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    // E' = e + 1023 = -511 + 1023 = 0x200

    // -(1 + 0b111... / 2^37) * 2^512
    v = decode(test::ByteBufferBuilder{
    //    MMMMMMMM    MMMMMMMM    MMMMMMMM    EEMMMMMM
        0b11111111, 0b11111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111
    //    EEEMMMMM    sEEEEEEE
    }.buffer, 5);
    ASSERT_EQUAL(0b1101111111111111111111111111111111111111111111111000000000000000ull, v);
    //             sEEEEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    // E' = e + 1023 = 512 + 1023 = 0x5FF

    // k = 6
    // r = 11
    // p = 44
    // e = E - 2^(r-1) + 1 = E - 1023

    // 2^-1023
    v = decode(test::ByteBufferBuilder{
    //    MMMMMMMM    MMMMMMMM    MMMMMMMM    MMMMMMMM
        0b00000000, 0b00000000, 0b00000000, 0b00000000,
        0b00000000, 0b00000000, 0b00000000
    //    MMMMMMMM    EEEEMMMM    sEEEEEEE
    }.buffer, 6);
    ASSERT_EQUAL(0b0000000000000000000000000000000000000000000000000000000000000000ull, v);
    //             sEEEEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    // E' = e + 1023 = -1023 + 1023 = 0x000

    // -(1 + 0b111... / 2^44) * 2^1024
    v = decode(test::ByteBufferBuilder{
    //    MMMMMMMM    MMMMMMMM    MMMMMMMM    EEMMMMMM
        0b11111111, 0b11111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11111111
    //    MMMMMMMM    EEEEMMMM    sEEEEEEE
    }.buffer, 6);
    ASSERT_EQUAL(0b1111111111111111111111111111111111111111111111111111111100000000ull, v);
    //             sEEEEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    // E' = e + 1023 = 1024 + 1023 = 0x7FF

    // k = 7
    // r = 11
    // p = 52

    v = decode(test::ByteBufferBuilder{
        0b00000000, 0b00000000, 0b00000000, 0b00000000,
        0b00000000, 0b00000000, 0b00000000, 0b00000000
    }.buffer, 7);
    ASSERT_EQUAL(0ull, v);

    v = decode(test::ByteBufferBuilder{
        0b11111111, 0b11111111, 0b11111111, 0b11111111,
        0b11111111, 0b11111111, 0b11111111, 0b11111111
    }.buffer, 7);
    ASSERT_EQUAL(UINT64_MAX, v);
}


static void testConvertBinaryRational32bTo64b() {
    const auto convert = dbor::Encoding::convertBinaryRational32bTo64b;

    // 32 bit:
    //
    //   r = 8
    //   p = 23
    //   e = E - 2^(r-1) + 1 = E - 127

    // E' = e + 1023 = -127 + 1023 = 0x380:
    //             sEEEEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    ASSERT_EQUAL(0b0011100000000000000000000000000000000000000000000000000000000000ull,
                 convert(0b00000000000000000000000000000000ul));
    // 2^-127              sEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM

    // E' = e + 1023 = 128 + 1023 = 0x47F:
    //             sEEEEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    ASSERT_EQUAL(0b1100011111111111111111111111111111100000000000000000000000000000ull,
                 convert(0b11111111111111111111111111111111ul));
    //                     sEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM
    // -(1 + 0b11111111111111111111111 / 2^23) * 2^128
}


void testEncoding() {
    testSizeOfTokenFromFirstByte();
    testSizeOfValueIn();

    testDecodeNaturalTokenData16();
    testDecodeNaturalTokenData32();
    testDecodeNaturalTokenData64();

    testEncodeNaturalTokenData16();
    testEncodeNaturalTokenData32();
    testEncodeNaturalTokenData64();

    testDecodeBinaryRationalTokenDataAs32b();
    testDecodeBinaryRationalTokenDataAs64b();
    testConvertBinaryRational32bTo64b();
}
