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


std::size_t Encoding::sizeOfValueIn(const std::uint8_t *p, std::size_t capacity) noexcept {
    if (!capacity)
        return 0;

    const std::uint8_t firstByte = *p;
    const std::size_t sizeOfFirst = sizeOfTokenFromFirstByte(firstByte); // > 0

    if (firstByte < 0x40 || firstByte >= 0xF0 || ((firstByte & 0xF8) == 0xC8))
        // IntegerValue or BinaryRationalValue or NumberlikeValue or NoneValue (or reserved)
        return sizeOfFirst;

    if (firstByte < 0xD0) {
        // StringValue or ContainerValue
        std::size_t offset = sizeOfFirst;

        if (firstByte < 0xC0) {
            const std::size_t m = firstByte & 0x1F;
            if (m < 0x18)
                return sizeOfFirst + m;
            offset += 23;
        }

        std::size_t n;
        if (sizeOfFirst <= capacity && decodeNaturalTokenData(n, &p[1], sizeOfFirst - 1u, offset) == ErrorCode::OK)
            return n;

        return 0u;  // too large for std::size_t
    }

    // DecimalRationalValue
    if (sizeOfFirst >= capacity)
        return 0;

    const std::uint8_t firstByteOfSecond = p[sizeOfFirst];
    if (firstByteOfSecond > 0x40)
        return sizeOfFirst;  // ill-formed - ignore second token

    return sizeOfFirst + sizeOfTokenFromFirstByte(firstByteOfSecond);
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


