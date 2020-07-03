// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>


dbor::ErrorCode Encoding::decodeNaturalTokenData(std::uint32_t &value,
                                                 const std::uint8_t *p, std::size_t n,
                                                 std::uint32_t offset) noexcept
{
    return impl::decodeNaturalTokenData<std::uint32_t, std::uint_fast32_t>(value, p, n, offset);
}


dbor::ErrorCode Encoding::decodeNaturalTokenData(std::uint64_t &value,
                                                 const std::uint8_t *p, std::size_t n,
                                                 std::uint32_t offset) noexcept
{
    static constexpr std::uint_fast32_t ONE_PER_BYTE = 0x01010101ull;

    value = 0u;
    if (n - 1u > 8u - 1u)
        return ErrorCode::OUT_OF_RANGE;

    // 0 < n <= 8
    uint_fast32_t high;
    uint_fast32_t low;
    if (n > 4u) {
        high = impl::readUintLeFromBuffer<std::uint_fast32_t>(p + 4u, n - 4u);
        low = impl::readUintLeFromBuffer<std::uint_fast32_t>(p, 4u);
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
        low = impl::readUintLeFromBuffer<std::uint_fast32_t>(p, n);
        const uint32_t dLow = (ONE_PER_BYTE >> (8u * (4u - n))) + offset;
        if (low > UINT32_MAX - dLow)
            high++;
        low += dLow;
    }

    value = (static_cast<std::uint64_t>(high) << 32u) | low;
    return ErrorCode::OK;
}


std::size_t Encoding::encodeNaturalTokenData(const std::uint16_t &value,
                                             std::uint8_t *p, std::size_t capacity) noexcept
{
    return encodeNaturalTokenData(static_cast<std::uint32_t>(value), p, capacity);
}


std::size_t Encoding::encodeNaturalTokenData(const std::uint32_t &value,
                                             std::uint8_t *p, std::size_t capacity) noexcept
{
    std::uint_fast32_t v = value;
    if (v == 0)
        return 0;

    std::size_t n = 0;
    std::uint_fast32_t encodedValueBe = 0;
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
    std::uint_fast32_t high = value >> 32u;

    if (!high)
        return encodeNaturalTokenData(low, p, capacity);

    // n will be >= 4

    std::uint32_t encodedLowLe = low - ONE_PER_BYTE;
    if (low < ONE_PER_BYTE)
        high--;

    std::size_t nHigh = 0;
    std::uint_fast32_t encodedHighBe = 0;
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
