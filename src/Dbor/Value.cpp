// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "Dbor/Value.hpp"
#include <limits>
#include <type_traits>
#include "Dbor/Conf.hpp"
#include "Dbor/Encoding.hpp"

// Design decisions:
// - Optimize for use case where get() is called on most dbor::Value instances
// - Do not call Encoding::sizeOfValueIn() after construction
// - Avoid type detection, use duck typing approach
// - Do not access bytes outside buffer_[0] ... buffer_[size_ - 1] even if buffer changes between
//   calls of methods (i.e.: do not rely on isComplete_ for boundary checking)

using namespace dbor;


Value::Value() noexcept
    : buffer_(nullptr)
    , size_(0)
    , isComplete_(false)
{
}


Value::Value(const uint8_t *buffer, std::size_t capacity) noexcept {
    if (buffer && capacity) {
        buffer_ = buffer;
        size_ = Encoding::sizeOfValueIn(buffer, capacity);
        isComplete_ = size_ != 0 && size_ <= capacity;
        if (!isComplete_)
            size_ = capacity;
    } else {
        buffer_ = nullptr;
        size_ = 0;
        isComplete_ = false;
    }
}


bool Value::isNone() const noexcept {
    return buffer_ && buffer_[0] == static_cast<std::uint8_t>(Encoding::SingleByteValue::NONE);
}


bool Value::isNumberlike() const noexcept {
    return buffer_
        && buffer_[0] >= static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_ZERO)
        && buffer_[0] != static_cast<std::uint8_t>(Encoding::SingleByteValue::NONE);
}


bool Value::isNumber() const noexcept {
    return buffer_ && (buffer_[0] < 0x40 || (buffer_[0] >= 0xC8 && buffer_[0] < 0xF0));
}


bool Value::isString() const noexcept {
    return buffer_ && buffer_[0] >= 0x40 && buffer_[0] < 0x80;
}


bool Value::isContainer() const noexcept {
    return buffer_ && buffer_[0] >= 0x80 && buffer_[0] < 0xC8;
}


namespace dbor::impl {

    template<typename T>
    ResultCode convertNumberlikeToInteger(T &value, std::uint8_t firstByte) {
        static_assert(std::numeric_limits<T>::is_integer, "");

        value = 0;

        if (firstByte >= 0xFC) {
            switch (firstByte) {
            case static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_ZERO):
                return ResultCode::APPROX_IMPRECISE;
            case static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_INF):
                value = std::numeric_limits<T>::min();
                return ResultCode::APPROX_EXTREME;
            case static_cast<std::uint8_t>(Encoding::SingleByteValue::INF):
                value = std::numeric_limits<T>::max();
                return ResultCode::APPROX_EXTREME;
            case static_cast<std::uint8_t>(Encoding::SingleByteValue::NONE):
            default:
                return ResultCode::NO_OBJECT;
            }
        }

        return ResultCode::INCOMPATIBLE;
    }

    template<typename T>  // T: e.g. std::uint32_t
    static ResultCode getUnsignedInteger(T &value,
                                         const std::uint8_t *buffer, std::size_t size)
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

            if (!Encoding::decodeNaturalTokenData(value, &buffer[1], n, 23)) {
                value = std::numeric_limits<T>::max();
                return ResultCode::APPROX_EXTREME;
            }

            return ResultCode::OK;
        }

        if (firstByte < 0x40)
            // negative IntegerValue
            return ResultCode::APPROX_EXTREME;

        return convertNumberlikeToInteger(value, firstByte);
    }


    template<typename T>  // T: e.g. std::int32_t
    static ResultCode getSignedInteger(T &value, const std::uint8_t *buffer, std::size_t size) {
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
                if (u > static_cast<U>(std::numeric_limits<T>::max())) {
                    value = std::numeric_limits<T>::max();
                    return ResultCode::APPROX_EXTREME;
                }
                value = static_cast<T>(u);
            } else {
                // negative
                if (u >= -static_cast<U>(std::numeric_limits<T>::min())) {
                    value = std::numeric_limits<T>::min();
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

        return impl::convertNumberlikeToInteger(value, firstByte);
    }


    template<typename T>  // T: e.g. std::uint8_t
    ResultCode getUnsignedIntegerFromBigger(T &value,
                                            const std::uint8_t *buffer, std::size_t size)
    {
        static_assert(std::numeric_limits<T>::is_integer, "");
        static_assert(!std::numeric_limits<T>::is_signed, "");

        #if DBOR_HAS_FAST_64BIT_ARITH
            std::uint32_t v;
        #else
            std::uint64_t v;
        #endif

        ResultCode e = getUnsignedInteger(v, buffer, size);
        static_assert(sizeof(value) <= sizeof(v), "");

        if (v > std::numeric_limits<T>::max()) {
            v = std::numeric_limits<T>::max();
            e = ResultCode::APPROX_EXTREME;
        }

        value = v;
        return e;
    }


    template<typename T>  // T: e.g. std::int8_t
    ResultCode getSignedIntegerFromBigger(T &value,
                                          const std::uint8_t *buffer, std::size_t size)
    {
        static_assert(std::numeric_limits<T>::is_integer, "");
        static_assert(std::numeric_limits<T>::is_signed, "");

        #if DBOR_HAS_FAST_64BIT_ARITH
            std::int32_t v;
        #else
            std::int64_t v;
        #endif

        ResultCode e = getSignedInteger(v, buffer, size);
        if (v > std::numeric_limits<T>::max()) {
            v = std::numeric_limits<T>::max();
            e = ResultCode::APPROX_EXTREME;
        } else if (v < std::numeric_limits<T>::min()) {
            v = std::numeric_limits<T>::min();
            e = ResultCode::APPROX_EXTREME;
        }

        value = v;
        return e;
    }

}


ResultCode Value::get(std::uint8_t &value) const noexcept {
    return impl::getUnsignedIntegerFromBigger(value, buffer_, size_);
}


ResultCode Value::get(std::uint16_t &value) const noexcept {
    return impl::getUnsignedIntegerFromBigger(value, buffer_, size_);
}


ResultCode Value::get(std::uint32_t &value) const noexcept {
    #if DBOR_HAS_FAST_64BIT_ARITH
        return impl::getUnsignedIntegerFromBigger(value, buffer_, size_);
    #else
        return impl::getUnsignedInteger(value, buffer_, size_);
    #endif
}


ResultCode Value::get(std::uint64_t &value) const noexcept {
    return impl::getUnsignedInteger(value, buffer_, size_);
}


ResultCode Value::get(std::int8_t &value) const noexcept {
    return impl::getSignedIntegerFromBigger(value, buffer_, size_);
}


ResultCode Value::get(std::int16_t &value) const noexcept {
    return impl::getSignedIntegerFromBigger(value, buffer_, size_);
}


ResultCode Value::get(std::int32_t &value) const noexcept {
    #if DBOR_HAS_FAST_64BIT_ARITH
        return impl::getSignedIntegerFromBigger(value, buffer_, size_);
    #else
        return impl::getSignedInteger(value, buffer_, size_);
    #endif
}


ResultCode Value::get(std::int64_t &value) const noexcept {
    return impl::getSignedInteger(value, buffer_, size_);
}


ResultCode Value::get(const std::uint8_t *&bytes, std::size_t &stringSize) const noexcept {
    bytes = nullptr;
    stringSize = 0;

    if (!isComplete_)
        return ResultCode::INCOMPLETE;

    if (buffer_[0] == static_cast<std::uint8_t>(Encoding::SingleByteValue::NONE))
        return ResultCode::NO_OBJECT;

    if (buffer_[0] < 0x40 || buffer_[0] >= 0x60)
        return ResultCode::INCOMPATIBLE;

    // ByteStringValue
    std::size_t sizeOfFirstToken = Encoding::sizeOfTokenFromFirstByte(buffer_[0]);
    if (size_ < sizeOfFirstToken)
        return ResultCode::INCOMPLETE;

    stringSize = size_ - sizeOfFirstToken;
    bytes = &buffer_[sizeOfFirstToken];
    return ResultCode::OK;
}


ResultCode Value::get(String &value, std::size_t maxSize) const noexcept {
    // maxSize limits the number of instructions for string.check()

    value = String();

    if (!isComplete_)
        return ResultCode::INCOMPLETE;

    if (buffer_[0] == static_cast<std::uint8_t>(Encoding::SingleByteValue::NONE))
        return ResultCode::NO_OBJECT;

    if (buffer_[0] < 0x60 || buffer_[0] >= 0x80)
        return ResultCode::INCOMPATIBLE;

    // Utf8StringValue
    std::size_t sizeOfFirstToken = Encoding::sizeOfTokenFromFirstByte(buffer_[0]);
    if (size_ < sizeOfFirstToken)
        return ResultCode::INCOMPLETE;

    std::size_t stringSize = size_ - sizeOfFirstToken;
    const std::uint8_t *p = &buffer_[sizeOfFirstToken];
    ResultCode r = ResultCode::OK;

    if (stringSize > maxSize) {
        // find beginning of truncated code point: is well-formed after truncation if it was before
        stringSize = String::offsetOfLastCodepointIn(p, maxSize + 1);  // maxSize + 1 <= stringSize
        stringSize = stringSize <= maxSize ? stringSize : maxSize;
        r = ResultCode::APPROX_EXTREME;
    }

    value = String(p, stringSize);
    return r;
}


ResultCode Value::get(float &value) const noexcept {
    // C++:2011: "True if and only if the type adheres to IEC 559 standard.
    // International Electrotechnical Commission standard 559 is the same as IEEE 754."
    static_assert(std::numeric_limits<float>::is_iec559, "");
    static_assert(std::numeric_limits<float>::has_denorm == std::denorm_present, "");
    static_assert(sizeof(float) == 4, "");

    value = std::numeric_limits<float>::quiet_NaN();

    if (!isComplete_)
        return ResultCode::INCOMPLETE;

    if ((buffer_[0] & 0xF8) == 0xC8) {
        // BinaryRationalValue
        const std::uint_fast8_t k = buffer_[0] & 7;
        if (k >= size_)
            return ResultCode::INCOMPLETE;

        std::uint64_t v64 = Encoding::decodeBinaryRationalTokenData(&buffer_[1], k);

        const std::uint64_t v64WithoutSign = v64 & ((1ull << 63) - 1u);
        if (k == 7 && !v64WithoutSign)
            return ResultCode::ILLFORMED;

        union {
            float f32;
            std::uint32_t v32;
        };
        static_assert(sizeof(f32) == sizeof(v32), "");

        int absDir;
        v32 = Encoding::convertBinaryRational64ToBinary32(v64, absDir);
        value = f32;

        if (absDir > 0)
            return ResultCode::APPROX_EXTREME;
        if (absDir < 0)
            return ResultCode::APPROX_IMPRECISE;
        return ResultCode::OK;
    }

    switch (buffer_[0]) {
        case 0x00:
            value = 0.0f;
            return ResultCode::OK;
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::NONE):
            return ResultCode::NO_OBJECT;
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_ZERO):
            // C+:2011 is not clear about -0.0 ("the result is the negation of its operand"),
            // so avoid it
            static constexpr float MINUS_ZERO = -1.0f / std::numeric_limits<float>::infinity();
            value = MINUS_ZERO;
            return ResultCode::OK;
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_INF):
            value = -std::numeric_limits<float>::infinity();
            return ResultCode::OK;
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::INF):
            value = std::numeric_limits<float>::infinity();
            return ResultCode::OK;
        default:
            break;
    }

    return ResultCode::INCOMPATIBLE;
}


ResultCode Value::get(double &value) const noexcept {
    // C++:2011: "True if and only if the type adheres to IEC 559 standard.
    // International Electrotechnical Commission standard 559 is the same as IEEE 754."
    static_assert(std::numeric_limits<double>::is_iec559, "");
    static_assert(std::numeric_limits<double>::has_denorm == std::denorm_present, "");
    static_assert(sizeof(double) == 8, "");

    value = std::numeric_limits<double>::quiet_NaN();

    if (!isComplete_)
        return ResultCode::INCOMPLETE;

    if ((buffer_[0] & 0xF8) == 0xC8) {
        // BinaryRationalValue
        const std::uint_fast8_t k = buffer_[0] & 7;
        if (k >= size_)
            return ResultCode::INCOMPLETE;

        union {
            double f64;
            std::uint64_t v64;
        };
        static_assert(sizeof(f64) == sizeof(v64), "");

        v64 = Encoding::decodeBinaryRationalTokenData(&buffer_[1], k);
        if (k == 7) {
            const std::uint64_t v64WithoutSign = v64 & ((1ull << 63) - 1u);
            if (!v64WithoutSign)
                return ResultCode::ILLFORMED;
            if (v64WithoutSign >= 0x7FFull << 52u) {
                // exp is maximum (IEEE 754 uses this to represent +/-Infinity and NaN)
                value = v64 & (1ull << 63) ?
                    -std::numeric_limits<double>::infinity() :
                    std::numeric_limits<double>::infinity();
                return ResultCode::APPROX_EXTREME;
            }
        }

        value = f64;
        return ResultCode::OK;
    }

    switch (buffer_[0]) {
        case 0x00:
            value = 0.0;
            return ResultCode::OK;
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::NONE):
            return ResultCode::NO_OBJECT;
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_ZERO):
            // C+:2011 is not clear about -0.0 ("the result is the negation of its operand"),
            // so avoid it
            static constexpr double MINUS_ZERO = -1.0 / std::numeric_limits<double>::infinity();
            value = MINUS_ZERO;
            return ResultCode::OK;
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_INF):
            value = -std::numeric_limits<double>::infinity();
            return ResultCode::OK;
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::INF):
            value = std::numeric_limits<double>::infinity();
            return ResultCode::OK;
        default:
            break;
    }

    return ResultCode::INCOMPATIBLE;
}
