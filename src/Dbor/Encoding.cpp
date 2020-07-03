// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include <limits>
#include "Dbor/Encoding.hpp"

using namespace dbor;


#if !defined(DBOR_HAS_FAST_64BIT_ARITH)
    #if UINT_FAST32_SIZE >= 8
        #define DBOR_HAS_FAST_64BIT_ARITH 1
    #else
        #define DBOR_HAS_FAST_64BIT_ARITH 0
    #endif
#endif


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


namespace dbor::impl {

    // Returns n modulo 2^(8 sizeof(T)), where n is the unsigned integer with the
    // little-endian representation p[0] ... p[n - 1].
    template<typename T>  // T: uint32_t or uint64_t
    static T readUintLeFromBuffer(const std::uint8_t *p, std::size_t n) noexcept {
        static_assert(std::numeric_limits<T>::is_integer, "");
        static_assert(!std::numeric_limits<T>::is_signed, "");

        T v = 0u;
        p += n - 1u;
        while (n-- > 0u) {
            v = (v << 8u) | *p--;
        }
        return v;
    }

    // If n = 0 or n > sizeof(value): Returns ErrorCode::OUT_OF_RANGE and value = 0.
    // If 0 < n <= sizeof(value) and offset <= 0xFEFEFEFE:
    //    a) Returns ErrorCode::OK and value = v + offset,
    //       where <b, p[0], ... , p[n - 1]> = NaturalToken(v),
    //       if v + offset <= std::numeric_limits<T>::max().
    //    b) Returns ErrorCode::OUT_OF_RANGE and value = 0 otherwise.
    // T: uint32_t or uint64_t, F: uint_fast32_t or uint_fast64_t
    template<typename T, typename F=T>
    dbor::ErrorCode decodeNaturalTokenData(T &value, const std::uint8_t *p, std::size_t n,
                                           std::uint32_t offset) noexcept
    {
        static_assert(std::numeric_limits<T>::is_integer, "");
        static_assert(!std::numeric_limits<T>::is_signed, "");
        static_assert(std::numeric_limits<F>::is_integer, "");
        static_assert(!std::numeric_limits<F>::is_signed, "");
        static_assert(sizeof(F) >= sizeof(T), "");

        static_assert(sizeof(value) <= 8, "");
        static constexpr F ONE_PER_BYTE = static_cast<T>(0x0101010101010101ull);

        if (n - 1u > sizeof(value) - 1u) {
            value = 0u;
            return ErrorCode::OUT_OF_RANGE;
        }

        F v = readUintLeFromBuffer<F>(p, n);  // 0 < n <= sizeof(value)
        const F d = (ONE_PER_BYTE >> (8u * (sizeof(value) - n))) + offset;
        if (v <= std::numeric_limits<T>::max() - d) {
            value = v + d;
            return ErrorCode::OK;
        }

        value = 0u;
        return ErrorCode::OUT_OF_RANGE;
    }

}


#if DBOR_HAS_FAST_64BIT_ARITH
    #include "Encoding_64b.cpp.inc"
#else
    #include "Encoding_32b.cpp.inc"
#endif
