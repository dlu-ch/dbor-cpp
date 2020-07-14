// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include <limits>
#include "Dbor/Conf.hpp"
#include "Dbor/Encoding.hpp"

using namespace dbor;


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
        if (sizeOfFirst <= capacity && decodeNaturalTokenData(n, &p[1], sizeOfFirst - 1u, offset))
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


bool Encoding::decodeNaturalTokenData(std::uint16_t &value,
                                      const std::uint8_t *p, std::size_t n,
                                      std::uint32_t offset) noexcept
{
    std::uint32_t v;
    bool isOk = decodeNaturalTokenData(v, p, n, offset);
    if (v > UINT16_MAX) {
        v = 0u;
        isOk = false;
    }
    value = v;
    return isOk;
}


std::uint32_t Encoding::decodeBinaryRationalTokenData32(const std::uint8_t *p,
                                                        std::size_t k) noexcept
{
    // Returns BinaryRationalToken(23, 0, E', M') as an unsigned little-endian integer
    // that represents the same number as BinaryRationalToken(p, 0, E, M) = <h, p[0], ..., p[k]>
    // for p in {4, 10, 16, 23}, i.e. k < 4.
    // For k >= 4, the return value is unspecified (calling is safe, however).

    p += k;

    const bool isNeg = *p & 0x80;
    std::uint_fast32_t v = *p & 0x7F;
    for (std::size_t i = k; i > 0; i--)
        v = (v << 8u) | *--p;

    // s = 0 in v

    if (k < 3) {
        // k    r    p    2^(r-1)   left shift by (to align)
        // ---  ---  ---  --------  ------------------------
        // 0    3    4    4         19
        // 1    5    10   16        13
        // 2    7    16   64        7

        // represented value: 1.MMM... * 2^e with e = E + 1 - 2^(r - 1)

        // k    v
        // ---  --------------------------------
        //      MSB                          LSB
        // 0                            sEEEMMMM
        // 1                    sEEEEEMMMMMMMMMM
        // 2            sEEEEEEEMMMMMMMMMMMMMMMM

        std::uint32_t mantissaAligned = v << (19u - 6u * k);

        // k    mantissaAligned
        // ---  --------------------------------
        //      MSB                          LSB
        // 0          EEEMMMM0000000000000000000
        // 1        EEEEEMMMMMMMMMM0000000000000
        // 2      EEEEEEEMMMMMMMMMMMMMMMM0000000
        // 3    0EEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM  (result)

        std::uint_fast32_t exp = (mantissaAligned >> 23u) + 128u - (1u << (2u * (k + 1u)));
        v = (mantissaAligned & ((1ul << 23u) - 1u)) | (exp << 23u);  // with s = 0
    }

    if (isNeg)
        v |= 0x80000000ul;

    return v;
    // like IEEE-754:2008 single precision (1 sign bit, 8 exponent bits, 23 mantissa bits), but
    // minimum and maximum value of exponent do not have special meaning (never denormalized,
    // never NaN, or +/-Infinity).
}


std::uint64_t Encoding::decodeBinaryRationalTokenData64(const std::uint8_t *p,
                                                        std::size_t k) noexcept
{
    // Returns BinaryRationalToken(52, o, E', M') as an unsigned little-endian integer
    // that represents the same number as BinaryRationalToken(52, o, E, M) = <h, p[0], ..., p[k]>
    // for p in {30, 37, 44, 52}, i.e. 4 <= k < 8.
    // For k < 4 or k >= 8, the return value is unspecified (calling is safe, however).

    p += k;

    const bool isNeg = *p & 0x80;
    std::uint_fast64_t v = *p & 0x7F;
    for (std::size_t i = k; i > 0; i--)
        v = (v << 8u) | *--p;

    // s = 0 in v

    if (k < 7) {
        // k    r    p    2^(r-1)   left shift by (to align)
        // ---  ---  ---  --------  ------------------------
        // 4    9    30   256       22
        // 5    10   37   512       15
        // 6    11   44   1024      8

        // represented value: 1.MMM... * 2^e with e = E + 1 - 2^(r - 1)

        // k    v
        // ---  -----------------------------------------------------------------
        //      MSB                                                           LSB
        // 4                            sEEEEEEE EEMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
        // 5                    sEEEEEEEEEEMMMMM MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
        // 6            sEEEEEEEEEEEMMMMMMMMMMMM MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

        std::uint_fast64_t mantissaAligned = v << (50u - 7u * k);

        // k    mantissaAligned
        // ---  -----------------------------------------------------------------
        //      MSB                                                           LSB
        // 4      sEEEEEEEEEMMMMMMMMMMMMMMMMMMMM MMMMMMMMMM0000000000000000000000
        // 5     sEEEEEEEEEEMMMMMMMMMMMMMMMMMMMM MMMMMMMMMMMMMMMMM000000000000000
        // 6    sEEEEEEEEEEEMMMMMMMMMMMMMMMMMMMM MMMMMMMMMMMMMMMMMMMMMMMM00000000
        // 7    sEEEEEEEEEEEMMMMMMMMMMMMMMMMMMMM MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM  (result)

        std::uint_fast32_t exp = (mantissaAligned >> 52u) + 1024u - (1u << (k + 4u));
        v = (mantissaAligned & ((1ull << 52u) - 1u)) | (static_cast<std::uint64_t>(exp) << 52u);
    }

    if (isNeg)
        v |= 1ull << 63;

    return v;
    // like IEEE-754:2008 double precision (1 sign bit, 11 exponent bits, 52 mantissa bits), but
    // maximum value of exponent does not have special meaning (never NaN, or +/-Infinity).
}


std::uint64_t Encoding::convertBinaryRational32ToBinary64(std::uint32_t value) noexcept {
    // Returns value as IEEE-754:2008 binary64 (1 sign bit, 11 exponent bits, 52 mantissa bits)
    // that represents the same number as 'value'.
    // 'value' like IEEE-754:2008 single precision (1 sign bit, 8 exponent bits, 23 mantissa bits),
    // but minimum and maximum value of exponent do not have special meaning (never denormalized,
    // never NaN, or +/-Infinity).
    //
    //     MSB          value           LSB
    //     sEEEEEEEEMMMMMMMMMMMMMMMMMMMMMMM
    //
    // represented number: 1.MMM... * 2^e with e = E - 127

    std::uint64_t v = static_cast<std::uint64_t>(value & 0x007FFFFFul) << (52u - 23u);  // mantissa
    std::uint32_t e = ((value >> 23u) & 0xFF) + (1023u - 127u);  // exponent
    if (value & 0x80000000ul)
        e |= 1u << 11u;
    v |= static_cast<std::uint64_t>(e) << 52u;

    return v;
}


std::uint32_t Encoding::convertBinaryRational64ToBinary32(std::uint64_t value,
                                                          int &absDir) noexcept
{
    // Returns value as IEEE-754:2008 binary32 (1 sign bit, 8 exponent bits, 23 mantissa bits)
    // that represents the same number as 'value'.
    // 'value' like IEEE-754:2008 double precision (1 sign bit, 11 exponent bits, 52 mantissa bits),
    // but maximum value of exponent does not have special meaning (never NaN, or +/-Infinity).
    //
    //     MSB                         value                             LSB
    //     sEEEEEEEEEEEMMMMMMMMMMMMMMMMMMMM MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM

    const std::uint_fast32_t expAndSign = value >> 52u;
    const std::uint_fast32_t exp = expAndSign & 0x7FF;
    const std::uint_fast32_t sign = expAndSign & 0x800u ? 0x80000000ul : 0u;

    // normalized IEEE-754 binary32: (1 + M / 2^23) * 2^e  with -126 <= e <= 127
    // denormalized IEEE-754 binary32: M / 2^23 * 2^-126

    if (exp > 1023u + 127u) {
        absDir = 1;  // magnitude too large to represent exactly
        return 0x7F800000ul | sign;  // +/- Infinity
    }

    std::uint_fast32_t mant = (value >> 29u) & 0x7FFFFFu;
    std::uint_fast32_t v = sign;
    bool imprecise = value & ((1ul << 29u) - 1u);

    if (exp >= 1023u - 126u) {
        // normalized number
        v |= mant | ((exp - (1023u - 127u)) << 23u);
    } else {
        // denormalized number

        // for smallest positive IEEE-754 binary32:
        // exp - 1023 = -126 - 23  =>  exp = 874

        const std::uint_fast16_t h = (1023u - 126u) - exp;  // 0 .. 897
        if (h < 24u) {
            mant |= 1ul << 23u;
            imprecise = imprecise || (mant & ((1ul << h) - 1u));
            v |= mant >> h;
        } else
            imprecise = true;
    }

    absDir = imprecise ? -1 : 0;
    return v;
}


std::uint64_t Encoding::decodeBinaryRationalTokenData(const std::uint8_t *p,
                                                      std::size_t k) noexcept
{
    if (k >= 4)
        return Encoding::decodeBinaryRationalTokenData64(p, k);

    std::uint32_t v = Encoding::decodeBinaryRationalTokenData32(p, k);
    return Encoding::convertBinaryRational32ToBinary64(v);
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
        while (n-- > 0u)
            v = (v << 8u) | *p--;

        return v;
    }

    // If n = 0 or n > sizeof(value): Returns false and value = 0.
    // If 0 < n <= sizeof(value) and offset <= 0xFEFEFEFE:
    //    a) Returns true and value = v + offset,
    //       where <b, p[0], ... , p[n - 1]> = NaturalToken(v),
    //       if v + offset <= std::numeric_limits<T>::max().
    //    b) Returns false and value = 0 otherwise.
    // T: uint32_t or uint64_t, F: uint_fast32_t or uint_fast64_t
    template<typename T, typename F=T>
    bool decodeNaturalTokenData(T &value, const std::uint8_t *p, std::size_t n,
                                std::uint32_t offset) noexcept
    {
        static_assert(std::numeric_limits<T>::is_integer, "");
        static_assert(!std::numeric_limits<T>::is_signed, "");
        static_assert(std::numeric_limits<F>::is_integer, "");
        static_assert(!std::numeric_limits<F>::is_signed, "");
        static_assert(sizeof(F) >= sizeof(T), "");

        static_assert(sizeof(value) <= sizeof(std::uint64_t), "");
        static constexpr F ONE_PER_BYTE = static_cast<T>(0x0101010101010101ull);

        if (n - 1u > sizeof(value) - 1u) {
            value = 0u;
            return false;
        }

        F v = readUintLeFromBuffer<F>(p, n);  // 0 < n <= sizeof(value)
        const F d = (ONE_PER_BYTE >> (8u * (sizeof(value) - n))) + offset;
        if (v <= std::numeric_limits<T>::max() - d) {
            value = v + d;
            return true;
        }

        value = 0u;
        return false;
    }

}


#if DBOR_HAS_FAST_64BIT_ARITH
    #include "Dbor/Encoding_64b.cpp.inc"
#else
    #include "Dbor/Encoding_32b.cpp.inc"
#endif
