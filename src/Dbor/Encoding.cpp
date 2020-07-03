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


static std::uint_fast32_t readUintFast32Le(const std::uint8_t *p, std::size_t n) noexcept {
    uint32_t v = 0u;
    p += n - 1u;
    while (n-- > 0u) {
        v = (v << 8u) | *p--;
    }
    return v;
}


dbor::ErrorCode Encoding::decodeNaturalTokenData(std::uint16_t &value,
                                                 const std::uint8_t *p, std::size_t n,
                                                 std::uint32_t offset) noexcept
{
    std::uint32_t v;
    ErrorCode e = decodeNaturalTokenData(v, p, n, offset);
    if (v > UINT16_MAX) {
        v = 0u;
        e = ErrorCode::OUT_OF_RANGE;
    }
    value = v;
    return e;
}


dbor::ErrorCode Encoding::decodeNaturalTokenData(std::uint32_t &value,
                                                 const std::uint8_t *p, std::size_t n,
                                                 std::uint32_t offset) noexcept
{
    // Return v + offset with <p[0], ... , p[n - 1]> = NaturalToken(v)
    // for 0 < n <= 4 and offset <= 0xFEFEFEFE.
    static constexpr std::uint_fast32_t ONE_PER_BYTE = 0x01010101ull;

    if (n - 1u > 4u - 1u) {
        value = 0u;
        return ErrorCode::OUT_OF_RANGE;
    }

    uint_fast32_t v = readUintFast32Le(p, n);  // 0 < n <= sizeof(value)
    const uint_fast32_t d = (ONE_PER_BYTE >> (8u * (4u - n))) + offset;
    if (v <= UINT32_MAX - d) {
        value = v + d;
        return ErrorCode::OK;
    }

    value = 0u;
    return ErrorCode::OUT_OF_RANGE;
}


dbor::ErrorCode Encoding::decodeNaturalTokenData(std::uint64_t &value,
                                                 const std::uint8_t *p, std::size_t n,
                                                 std::uint32_t offset) noexcept
{
    // Return v + offset with <p[0], ... , p[n - 1]> = NaturalToken(v)
    // for 0 < n <= 8 and offset <= 0xFEFEFEFE.
    static constexpr std::uint_fast32_t ONE_PER_BYTE = 0x01010101ull;

    value = 0u;
    if (n - 1u > 8u - 1u)
        return ErrorCode::OUT_OF_RANGE;

    // 0 < n <= 8
    uint_fast32_t high;
    uint_fast32_t low;
    if (n > 4u) {
        high = readUintFast32Le(p + 4u, n - 4u);
        low = readUintFast32Le(p, 4u);
        uint32_t dHigh = ONE_PER_BYTE >> (8u * (8u - n));
        uint32_t dLow = ONE_PER_BYTE + offset;
        if (low > UINT32_MAX - dLow)
            dHigh++;
        low += dLow;
        if (high > UINT32_MAX - dHigh)
            return ErrorCode::OUT_OF_RANGE;
        high += dHigh;
    } else {
        high = 0u;
        low = readUintFast32Le(p, n);
        const uint32_t dLow = (ONE_PER_BYTE >> (8u * (4u - n))) + offset;
        if (low > UINT32_MAX - dLow)
            high++;
        low += dLow;
    }

    value = (static_cast<std::uint64_t>(high) << 32u) | low;
    return ErrorCode::OK;
}


std::size_t Encoding::encodeNaturalTokenData(const std::uint32_t &value,
                                             std::uint8_t *p, std::size_t capacity) noexcept
{
    std::uint32_t v = value;
    if (v == 0)
        return 0;

    std::size_t n = 0;
    std::uint32_t encodedValueBe = 0;
    do {
        n++;
        encodedValueBe = (encodedValueBe << 8u) + (--v & 0xFF);
        v >>= 8u;
    } while (v);

    if (n > capacity)
        return 0;

    p += n - 1u;  // 0 < n <= capacity
    for (std::size_t i = n; i > 0; i--) {
        *p-- = encodedValueBe;
        encodedValueBe >>= 8u;
    }

    return n;
}


std::size_t Encoding::encodeNaturalTokenData(const std::uint64_t &value,
                                             std::uint8_t *p, std::size_t capacity) noexcept
{
    static constexpr std::uint32_t ONE_PER_BYTE = 0x01010101ull;

    std::uint32_t low = value;
    std::uint32_t high = value >> 32u;

    if (!high)
        return encodeNaturalTokenData(low, p, capacity);

    // n will be >= 4

    std::uint32_t encodedLowLe = low - ONE_PER_BYTE;
    if (low < ONE_PER_BYTE)
        high--;

    std::size_t nHigh = 0;
    std::uint32_t encodedHighBe = 0;
    while (high) {
        nHigh++;
        encodedHighBe = (encodedHighBe << 8u) + (--high & 0xFF);
        high >>= 8u;
    }

    std::size_t n = nHigh + 4u;
    if (n > capacity)
        return 0;

    for (std::size_t i = 4u; i > 0; i--) {
        *p++ = encodedLowLe;
        encodedLowLe >>= 8u;
    }

    p += nHigh - 1u;
    for (std::size_t i = nHigh; i > 0; i--) {
        *p-- = encodedHighBe;
        encodedHighBe >>= 8u;
    }

    return n;  // 0 < n <= 8
}
