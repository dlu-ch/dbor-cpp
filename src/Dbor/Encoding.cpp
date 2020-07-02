// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "Dbor/Encoding.hpp"

using namespace dbor;


std::uint_least8_t Encoding::sizeInfoFromFirstByte(std::uint8_t b) noexcept {
    // first token of value is NaturalToken(v) and v has to be added to ret & 0x1F,
    // where n is return value
    static constexpr std::uint_least8_t ADDITIONALLY_VALUE_OF_NATURAL_TOKEN = 0x40;

    // next token must by IntegerValue and is part of value
    static constexpr std::uint_least8_t FOLLOWED_BY_INTEGERTOKEN = 0x80;

    // TODO -> update DBOR spec. accordingly
    // 000xxxxx  IntegerValue
    // 001xxxxx  IntegerValue
    // 010xxxxx  ByteStringValue
    // 011xxxxx  Utf8StringValue
    // 100xxxxx  SequenceValue
    // 101xxxxx  DictionaryValue
    // 11000yyy  AllocatedValue
    // 11001yyy  BinaryRationalValue
    // 1101xyyy  DecimalRationalValue(..., e) with |e| > 8
    // 1110xeee  DecimalRationalValue(..., e) with |e| <= 8
    // 1111xxxx  MinimalToken

    if (b >= 0xF0)
        return 1;

    if (b < 0xC0) {
        // first token is IntegerToken
        std::uint_least8_t n = b & 0x1F;
        if (b < 0x40) {
            return n < 0x18 ? 1 : 2 + (n & 7);  // size of first token
        } else {
            return n < 0x18 ? 1 + n : (2 + 23 + (n & 7)) | ADDITIONALLY_VALUE_OF_NATURAL_TOKEN;
        }
    }

    if (b >= 0xE0)
        return 1 + FOLLOWED_BY_INTEGERTOKEN;  // DecimalRationalValue(..., e) with |e| <= 8

    uint_least8_t n = 2 + (b & 7);
    if (b < 0xC8)
        return n | ADDITIONALLY_VALUE_OF_NATURAL_TOKEN;  // AllocatedValue

    return b < 0xD0 ? n : n | FOLLOWED_BY_INTEGERTOKEN;
}
