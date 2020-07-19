// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>


#if DBOR_HAS_FAST_64BIT_ARITH
    typedef std::uint64_t uint_preferred32_t;
    typedef std::int64_t int_preferred32_t;
#else
    typedef std::uint32_t uint_preferred32_t;
    typedef std::int32_t int_preferred32_t;
#endif


static ResultCode convertNumberlikeToInteger(std::uint8_t firstByte) noexcept {
    if (firstByte >= 0xFC) {
        switch (firstByte) {
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_ZERO):
            return ResultCode::APPROX_IMPRECISE;
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_INF):
            return ResultCode::APPROX_EXTREME;
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::INF):
            return ResultCode::APPROX_EXTREME;
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::NONE):
        default:
            return ResultCode::NO_OBJECT;
        }
    }

    return ResultCode::INCOMPATIBLE;
}


template<typename T>  // T: e.g. std::uint32_t
static ResultCode getUnsignedInteger(T &value, const T maximumValue,
                                     const std::uint8_t *buffer, std::size_t size) noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "");
    static_assert(!std::numeric_limits<T>::is_signed, "");

    value = 0;

    if (!size)
        return ResultCode::INCOMPLETE;

    const std::uint8_t firstByte = buffer[0];
    if (firstByte < 0x20) {
        // non-negative IntegerValue
        if (firstByte < 0x18) {
            value = firstByte;
            return ResultCode::OK;
        }

        std::size_t n = 1 + (firstByte & 7);
        if (size <= n)
            return ResultCode::INCOMPLETE;

        if (!Encoding::decodeNaturalTokenData(value, &buffer[1], n, 23)
            || value > maximumValue)
        {
            value = maximumValue;
            return ResultCode::APPROX_EXTREME;
        }

        return ResultCode::OK;
    }

    if (firstByte < 0x40)
        // negative IntegerValue
        return ResultCode::APPROX_EXTREME;

    ResultCode r = convertNumberlikeToInteger(firstByte);
    if (r != ResultCode::APPROX_EXTREME)
        return r;

    if (firstByte == static_cast<std::uint8_t>(Encoding::SingleByteValue::INF))
        value = maximumValue;

    return r;
}


template<typename T>  // T: e.g. std::uint8_t
typename std::enable_if<sizeof(T) < sizeof(impl::uint_preferred32_t),
                        ResultCode>::type
getUnsignedInteger(T &value, const std::uint8_t *buffer, std::size_t size) noexcept {
    static_assert(std::numeric_limits<T>::is_integer, "");
    static_assert(!std::numeric_limits<T>::is_signed, "");

    impl::uint_preferred32_t v;
    ResultCode r = getUnsignedInteger(
        v,
        static_cast<decltype(v)>(std::numeric_limits<T>::max()),
        buffer, size);

    value = v;
    return r;
}


template<typename T>  // T: e.g. std::uint64_t
typename std::enable_if<sizeof(T) >= sizeof(impl::uint_preferred32_t),
                        ResultCode>::type
getUnsignedInteger(T &value, const std::uint8_t *buffer, std::size_t size) noexcept {
    static_assert(std::numeric_limits<T>::is_integer, "");
    static_assert(!std::numeric_limits<T>::is_signed, "");
    return getUnsignedInteger(value, std::numeric_limits<T>::max(), buffer, size);
}


template<typename T>  // T: e.g. std::int32_t
static ResultCode getSignedInteger(T &value, const T minimumValue, const T maximumValue,
                                   const std::uint8_t *buffer, std::size_t size) noexcept
{
    static_assert(std::numeric_limits<T>::is_integer, "");
    static_assert(std::numeric_limits<T>::is_signed, "");
    typedef typename std::make_unsigned<T>::type U;

    value = 0;

    if (!size)
        return ResultCode::INCOMPLETE;

    const std::uint8_t firstByte = buffer[0];
    if (firstByte < 0x40) {
        // IntegerValue
        U u = firstByte & 0x1F;
        if (u >= 0x18) {
            std::size_t n = 1 + (firstByte & 7);
            if (size <= n)
                return ResultCode::INCOMPLETE;

            if (!Encoding::decodeNaturalTokenData(u, &buffer[1], n, 23))
                u = std::numeric_limits<U>::max();
        }

        // Beware: converting U to T with static_cast<T>(u) is undefined prior to C++:2014
        // if T cannot represent u.

        if (firstByte < 0x20) {
            // non-negative
            if (u > static_cast<U>(maximumValue)) {
                value = maximumValue;
                return ResultCode::APPROX_EXTREME;
            }
            value = static_cast<T>(u);
        } else {
            // negative
            if (u >= -static_cast<U>(minimumValue)) {
                value = minimumValue;
                return ResultCode::APPROX_EXTREME;
            }
            // T can represent -(u + 1)

            // https://stackoverflow.com/questions/13150449/efficient-unsigned-to-signed-cast-
            // avoiding-implementation-defined-behavior/13208789#13208789:
            // "Since C++11 inherits the <climits> macros from C99, INT_MIN is either
            // -INT_MAX or -INT_MAX-1 [...]".

            // INT_MAX - 1 <= |INT_MIN| - 1 <= INT_MAX:
            static_assert(-static_cast<U>(std::numeric_limits<T>::min()) > 0);
            static_assert(-static_cast<U>(std::numeric_limits<T>::min()) - 1
                          >= static_cast<U>(std::numeric_limits<T>::max()), "");
            static_assert(-static_cast<U>(std::numeric_limits<T>::min()) - 1
                          <= static_cast<U>(std::numeric_limits<T>::max()), "");

            // since T can represent -(u + 1), T can represent u:
            value = -static_cast<T>(u) - 1;
        }
        return ResultCode::OK;
    }

    ResultCode r = convertNumberlikeToInteger(firstByte);
    if (r != ResultCode::APPROX_EXTREME)
        return r;

    value =
        firstByte == static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_INF) ?
        minimumValue :
        maximumValue;
    return r;;
}


template<typename T>  // T: e.g. std::int8_t
typename std::enable_if<sizeof(T) < sizeof(impl::int_preferred32_t),
                        ResultCode>::type
getSignedInteger(T &value, const std::uint8_t *buffer, std::size_t size) noexcept {
    static_assert(std::numeric_limits<T>::is_integer, "");
    static_assert(std::numeric_limits<T>::is_signed, "");

    impl::int_preferred32_t v;
    ResultCode r = getSignedInteger(
        v,
        static_cast<decltype(v)>(std::numeric_limits<T>::min()),
        static_cast<decltype(v)>(std::numeric_limits<T>::max()),
        buffer, size);

    value = v;
    return r;
}


template<typename T>  // T: e.g. std::int64_t
typename std::enable_if<sizeof(T) >= sizeof(impl::int_preferred32_t),
                        ResultCode>::type
getSignedInteger(T &value, const std::uint8_t *buffer, std::size_t size) noexcept {
    static_assert(std::numeric_limits<T>::is_integer, "");
    static_assert(std::numeric_limits<T>::is_signed, "");
    return getSignedInteger(
        value, std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), buffer, size);
}
