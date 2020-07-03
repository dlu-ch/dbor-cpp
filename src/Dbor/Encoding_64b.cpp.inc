// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>


dbor::ErrorCode Encoding::decodeNaturalTokenData(std::uint32_t &value,
                                                 const std::uint8_t *p, std::size_t n,
                                                 std::uint32_t offset) noexcept
{
    std::uint64_t v;
    ErrorCode e = decodeNaturalTokenData(v, p, n, offset);
    if (v > UINT32_MAX) {
        v = 0u;
        e = ErrorCode::OUT_OF_RANGE;
    }
    value = v;
    return e;
}


dbor::ErrorCode Encoding::decodeNaturalTokenData(std::uint64_t &value,
                                                 const std::uint8_t *p, std::size_t n,
                                                 std::uint32_t offset) noexcept
{
    return impl::decodeNaturalTokenData<std::uint64_t, std::uint_fast64_t>(
        value, p, n, offset);
}


std::size_t Encoding::encodeNaturalTokenData(const std::uint16_t &value,
                                             std::uint8_t *p, std::size_t capacity) noexcept
{
    return encodeNaturalTokenData(static_cast<std::uint64_t>(value), p, capacity);
}


std::size_t Encoding::encodeNaturalTokenData(const std::uint32_t &value,
                                             std::uint8_t *p, std::size_t capacity) noexcept
{
    return encodeNaturalTokenData(static_cast<std::uint64_t>(value), p, capacity);
}


std::size_t Encoding::encodeNaturalTokenData(const std::uint64_t &value,
                                             std::uint8_t *p, std::size_t capacity) noexcept
{
    std::uint_fast64_t v = value;
    if (v == 0)
        return 0;

    std::size_t n = 0;
    std::uint_fast64_t encodedValueBe = 0;
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
