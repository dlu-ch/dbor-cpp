// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "Dbor/Value.hpp"
#include <limits>
#include <type_traits>
#include "Dbor/Conf.hpp"
#include "Dbor/Encoding.hpp"

// Design decisions:
// - Optimize for use case where getAsXXX() is called on most dbor::Value instances
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
    ResultCodes convertNumberlikeToInteger(T &value, std::uint8_t firstByte) {
        static_assert(std::numeric_limits<T>::is_integer, "");

        value = 0;

        if (firstByte >= 0xFC) {
            switch (firstByte) {
            case static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_ZERO):
                return ResultCodes::APPROX_PRECISION;
            case static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_INF):
                value = std::numeric_limits<T>::min();
                return ResultCodes::APPROX_RANGE;
            case static_cast<std::uint8_t>(Encoding::SingleByteValue::INF):
                value = std::numeric_limits<T>::max();
                return ResultCodes::APPROX_RANGE;
            case static_cast<std::uint8_t>(Encoding::SingleByteValue::NONE):
            default:
                return ResultCodes::NO_OBJECT;
            }
        }

        return ResultCodes::INCOMPATIBLE;
    }

    template<typename T>  // T: e.g. std::uint32_t
    static ResultCodes getAsUnsignedInteger(T &value,
                                            const std::uint8_t *buffer, std::size_t size)
    {
        static_assert(std::numeric_limits<T>::is_integer, "");
        static_assert(!std::numeric_limits<T>::is_signed, "");

        value = 0;

        if (!size)
            return ResultCodes::INCOMPLETE;

        const std::uint8_t firstByte = buffer[0];
        if (firstByte < 0x20) {
            // non-negative IntegerValue
            if (firstByte < 0x18) {
                value = firstByte;
                return ResultCodes::OK;
            }

            std::size_t n = 1 + (firstByte & 7);
            if (size <= n)
                return ResultCodes::INCOMPLETE;

            if (!Encoding::decodeNaturalTokenData(value, &buffer[1], n, 23)) {
                value = std::numeric_limits<T>::max();
                return ResultCodes::APPROX_RANGE;
            }

            return ResultCodes::OK;
        }

        if (firstByte < 0x40)
            // negative IntegerValue
            return ResultCodes::APPROX_RANGE;

        return convertNumberlikeToInteger(value, firstByte);
    }


    template<typename T>  // T: e.g. std::int32_t
    static ResultCodes getAsSignedInteger(T &value, const std::uint8_t *buffer, std::size_t size) {
        static_assert(std::numeric_limits<T>::is_integer, "");
        static_assert(std::numeric_limits<T>::is_signed, "");
        typedef typename std::make_unsigned<T>::type U;

        value = 0;

        if (!size)
            return ResultCodes::INCOMPLETE;

        const std::uint8_t firstByte = buffer[0];
        if (firstByte < 0x40) {
            // IntegerValue
            U u = firstByte & 0x1F;
            if (u >= 0x18) {
                std::size_t n = 1 + (firstByte & 7);
                if (size <= n)
                    return ResultCodes::INCOMPLETE;

                if (!Encoding::decodeNaturalTokenData(u, &buffer[1], n, 23))
                    u = std::numeric_limits<U>::max();
            }

            // Beware: converting U to T with static_cast<T>(u) is undefined prior to C++:2014
            // if T cannot represent u.

            if (firstByte < 0x20) {
                // non-negative
                if (u > static_cast<U>(std::numeric_limits<T>::max())) {
                    value = std::numeric_limits<T>::max();
                    return ResultCodes::APPROX_RANGE;
                }
                value = static_cast<T>(u);
            } else {
                // negative
                if (u >= -static_cast<U>(std::numeric_limits<T>::min())) {
                    value = std::numeric_limits<T>::min();
                    return ResultCodes::APPROX_RANGE;
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
            return ResultCodes::OK;
        }

        return impl::convertNumberlikeToInteger(value, firstByte);
    }


    template<typename T>  // T: e.g. std::uint8_t
    ResultCodes getAsUnsignedIntegerFromBigger(T &value,
                                              const std::uint8_t *buffer, std::size_t size)
    {
        static_assert(std::numeric_limits<T>::is_integer, "");
        static_assert(!std::numeric_limits<T>::is_signed, "");

        #if DBOR_HAS_FAST_64BIT_ARITH
            std::uint32_t v;
        #else
            std::uint64_t v;
        #endif

        ResultCodes e = getAsUnsignedInteger(v, buffer, size);
        static_assert(sizeof(value) <= sizeof(v), "");

        if (v > std::numeric_limits<T>::max()) {
            v = std::numeric_limits<T>::max();
            e = ResultCodes::APPROX_RANGE;
        }

        value = v;
        return e;
    }


    template<typename T>  // T: e.g. std::int8_t
    ResultCodes getAsSignedIntegerFromBigger(T &value,
                                            const std::uint8_t *buffer, std::size_t size)
    {
        static_assert(std::numeric_limits<T>::is_integer, "");
        static_assert(std::numeric_limits<T>::is_signed, "");

        #if DBOR_HAS_FAST_64BIT_ARITH
            std::int32_t v;
        #else
            std::int64_t v;
        #endif

        ResultCodes e = getAsSignedInteger(v, buffer, size);
        if (v > std::numeric_limits<T>::max()) {
            v = std::numeric_limits<T>::max();
            e = ResultCodes::APPROX_RANGE;
        } else if (v < std::numeric_limits<T>::min()) {
            v = std::numeric_limits<T>::min();
            e = ResultCodes::APPROX_RANGE;
        }

        value = v;
        return e;
    }

}


ResultCodes Value::getAsInteger(std::uint8_t &value) const noexcept {
    return impl::getAsUnsignedIntegerFromBigger(value, buffer_, size_);
}


ResultCodes Value::getAsInteger(std::uint16_t &value) const noexcept {
    return impl::getAsUnsignedIntegerFromBigger(value, buffer_, size_);
}


ResultCodes Value::getAsInteger(std::uint32_t &value) const noexcept {
    #if DBOR_HAS_FAST_64BIT_ARITH
        return impl::getAsUnsignedIntegerFromBigger(value, buffer_, size_);
    #else
        return impl::getAsUnsignedInteger(value, buffer_, size_);
    #endif
}


ResultCodes Value::getAsInteger(std::uint64_t &value) const noexcept {
    return impl::getAsUnsignedInteger(value, buffer_, size_);
}


ResultCodes Value::getAsInteger(std::int8_t &value) const noexcept {
    return impl::getAsSignedIntegerFromBigger(value, buffer_, size_);
}


ResultCodes Value::getAsInteger(std::int16_t &value) const noexcept {
    return impl::getAsSignedIntegerFromBigger(value, buffer_, size_);
}


ResultCodes Value::getAsInteger(std::int32_t &value) const noexcept {
    #if DBOR_HAS_FAST_64BIT_ARITH
        return impl::getAsSignedIntegerFromBigger(value, buffer_, size_);
    #else
        return impl::getAsSignedInteger(value, buffer_, size_);
    #endif
}


ResultCodes Value::getAsInteger(std::int64_t &value) const noexcept {
    return impl::getAsSignedInteger(value, buffer_, size_);
}


ResultCodes Value::getAsByteString(const std::uint8_t *&bytes,
                                   std::size_t &stringSize) const noexcept
{
    bytes = nullptr;
    stringSize = 0;

    if (!isComplete_)
        return ResultCodes::INCOMPLETE;

    if (buffer_[0] == static_cast<std::uint8_t>(Encoding::SingleByteValue::NONE))
        return ResultCodes::NO_OBJECT;

    if (buffer_[0] < 0x40 || buffer_[0] >= 0x60)
        return ResultCodes::INCOMPATIBLE;

    // ByteStringValue
    std::size_t sizeOfFirstToken = Encoding::sizeOfTokenFromFirstByte(buffer_[0]);
    if (size_ < sizeOfFirstToken)
        return ResultCodes::INCOMPLETE;

    stringSize = size_ - sizeOfFirstToken;
    bytes = &buffer_[sizeOfFirstToken];
    return ResultCodes::OK;
}


ResultCodes Value::getAsUtf8String(String &string, std::size_t maxSize) const noexcept {
    // maxSize limits the number of instructions for string.check()

    string = String();

    if (!isComplete_)
        return ResultCodes::INCOMPLETE;

    if (buffer_[0] == static_cast<std::uint8_t>(Encoding::SingleByteValue::NONE))
        return ResultCodes::NO_OBJECT;

    if (buffer_[0] < 0x60 || buffer_[0] >= 0x80)
        return ResultCodes::INCOMPATIBLE;

    // Utf8StringValue
    std::size_t sizeOfFirstToken = Encoding::sizeOfTokenFromFirstByte(buffer_[0]);
    if (size_ < sizeOfFirstToken)
        return ResultCodes::INCOMPLETE;

    std::size_t stringSize = size_ - sizeOfFirstToken;
    const std::uint8_t *p = &buffer_[sizeOfFirstToken];
    ResultCodes r = ResultCodes::OK;

    if (stringSize > maxSize) {
        // find beginning of truncated code point: is well-formed after truncation if it was before
        stringSize = String::offsetOfLastCodepointIn(p, maxSize + 1);  // maxSize + 1 <= stringSize
        stringSize = stringSize <= maxSize ? stringSize : maxSize;
        r = ResultCodes::APPROX_RANGE;
    }

    string = String(p, stringSize);
    return r;
}
