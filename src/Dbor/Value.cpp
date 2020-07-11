// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "Dbor/Value.hpp"
#include <limits>
#include <type_traits>
#include "Dbor/Conf.hpp"
#include "Dbor/Encoding.hpp"

using namespace dbor;


Value::Value() noexcept
    : buffer_(nullptr)
    , size_(0)
{
}


Value::Value(const uint8_t *buffer, std::size_t capacity) noexcept {
    if (buffer && capacity) {
        size_ = Encoding::sizeOfValueIn(buffer, capacity);
        if (size_ == 0 || size_ > capacity)
            size_ = capacity;
        buffer_ = buffer;
    } else {
        buffer_ = nullptr;
        size_ = 0;
    }
}


bool Value::isNone() const noexcept {
    return size_ && buffer_[0] == static_cast<std::uint8_t>(Encoding::SingleByteValue::NONE);
}


bool Value::isNumberlike() const noexcept {
    return size_
        && buffer_[0] >= static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_ZERO)
        && buffer_[0] != static_cast<std::uint8_t>(Encoding::SingleByteValue::NONE);
}


bool Value::isNumber() const noexcept {
    return size_ && (buffer_[0] < 0x40 || (buffer_[0] >= 0xC8 && buffer_[0] < 0xF0));
}


bool Value::isString() const noexcept {
    return size_ && buffer_[0] >= 0x40 && buffer_[0] < 0x80;
}


bool Value::isContainer() const noexcept {
    return size_ && buffer_[0] >= 0x80 && buffer_[0] < 0xC8;
}


namespace dbor::impl {

    template<typename T>
    ErrorCode convertNumberlikeToInteger(T &value, std::uint8_t firstByte) {
        static_assert(std::numeric_limits<T>::is_integer, "");

        value = 0;

        if (firstByte >= 0xFC) {
            switch (firstByte) {
            case static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_ZERO):
                return ErrorCode::OK_IMPRECISE;
            case static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_INF):
                value = std::numeric_limits<T>::min();
                return ErrorCode::OUT_OF_RANGE;
            case static_cast<std::uint8_t>(Encoding::SingleByteValue::INF):
                value = std::numeric_limits<T>::max();
                return ErrorCode::OUT_OF_RANGE;
            case static_cast<std::uint8_t>(Encoding::SingleByteValue::NONE):
            default:
                return ErrorCode::NO_OBJECT;
            }
        }

        return ErrorCode::INCOMPATIBLE;
    }

    template<typename T>  // T: e.g. std::uint32_t
    static ErrorCode getAsUnsignedInteger(T &value, const std::uint8_t *buffer, std::size_t size) {
        static_assert(std::numeric_limits<T>::is_integer, "");
        static_assert(!std::numeric_limits<T>::is_signed, "");

        value = 0;

        if (!size)
            return ErrorCode::INCOMPLETE;

        const std::uint8_t firstByte = buffer[0];
        if (firstByte < 0x20) {
            // non-negative IntegerValue
            if (firstByte < 0x18) {
                value = firstByte;
                return ErrorCode::OK_PRECISE;
            }

            std::size_t n = 1 + (firstByte & 7);
            if (size <= n)
                return ErrorCode::INCOMPLETE;

            if (!Encoding::decodeNaturalTokenData(value, &buffer[1], n, 23)) {
                value = std::numeric_limits<T>::max();
                return ErrorCode::OUT_OF_RANGE;
            }

            return ErrorCode::OK_PRECISE;
        }

        if (firstByte < 0x40)
            // negative IntegerValue
            return ErrorCode::OUT_OF_RANGE;

        return convertNumberlikeToInteger(value, firstByte);
    }


    template<typename T>  // T: e.g. std::int32_t
    static ErrorCode getAsSignedInteger(T &value, const std::uint8_t *buffer, std::size_t size) {
        static_assert(std::numeric_limits<T>::is_integer, "");
        static_assert(std::numeric_limits<T>::is_signed, "");
        typedef typename std::make_unsigned<T>::type U;

        value = 0;

        if (!size)
            return ErrorCode::INCOMPLETE;

        const std::uint8_t firstByte = buffer[0];
        if (firstByte < 0x40) {
            // IntegerValue
            U u = firstByte & 0x1F;
            if (u >= 0x18) {
                std::size_t n = 1 + (firstByte & 7);
                if (size <= n)
                    return ErrorCode::INCOMPLETE;

                if (!Encoding::decodeNaturalTokenData(u, &buffer[1], n, 23))
                    u = std::numeric_limits<U>::max();
            }

            // Beware: converting U to T with static_cast<T>(u) is undefined prior to C++:2014
            // if T cannot represent u.

            if (firstByte < 0x20) {
                // non-negative
                if (u > static_cast<U>(std::numeric_limits<T>::max())) {
                    value = std::numeric_limits<T>::max();
                    return ErrorCode::OUT_OF_RANGE;
                }
                value = static_cast<T>(u);
            } else {
                // negative
                if (u >= -static_cast<U>(std::numeric_limits<T>::min())) {
                    value = std::numeric_limits<T>::min();
                    return ErrorCode::OUT_OF_RANGE;
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
            return ErrorCode::OK_PRECISE;
        }

        return impl::convertNumberlikeToInteger(value, firstByte);
    }


    template<typename T>  // T: e.g. std::uint8_t
    ErrorCode getAsUnsignedIntegerFromBigger(T &value,
                                             const std::uint8_t *buffer, std::size_t size)
    {
        static_assert(std::numeric_limits<T>::is_integer, "");
        static_assert(!std::numeric_limits<T>::is_signed, "");

        std::uint32_t v;
        ErrorCode e = getAsUnsignedInteger(v, buffer, size);
        static_assert(sizeof(value) <= sizeof(v), "");

        if (v > std::numeric_limits<T>::max()) {
            v = std::numeric_limits<T>::max();
            e = ErrorCode::OUT_OF_RANGE;
        }

        value = v;
        return e;
    }


    template<typename T>  // T: e.g. std::int8_t
    ErrorCode getAsSignedIntegerFromBigger(T &value, const std::uint8_t *buffer, std::size_t size) {
        static_assert(std::numeric_limits<T>::is_integer, "");
        static_assert(std::numeric_limits<T>::is_signed, "");

        std::int32_t v;
        ErrorCode e = getAsSignedInteger(v, buffer, size);
        if (v > std::numeric_limits<T>::max()) {
            v = std::numeric_limits<T>::max();
            e = ErrorCode::OUT_OF_RANGE;
        } else if (v < std::numeric_limits<T>::min()) {
            v = std::numeric_limits<T>::min();
            e = ErrorCode::OUT_OF_RANGE;
        }

        value = v;
        return e;
    }


}


ErrorCode Value::getAsInteger(std::uint8_t &value) const noexcept {
    return impl::getAsUnsignedIntegerFromBigger(value, buffer_, size_);
}


ErrorCode Value::getAsInteger(std::uint16_t &value) const noexcept {
    return impl::getAsUnsignedIntegerFromBigger(value, buffer_, size_);
}


ErrorCode Value::getAsInteger(std::uint32_t &value) const noexcept {
    #if DBOR_HAS_FAST_64BIT_ARITH
        return impl::getAsUnsignedIntegerFromBigger(value, buffer_, size_);
    #else
        return impl::getAsUnsignedInteger(value, buffer_, size_);
    #endif
}


ErrorCode Value::getAsInteger(std::uint64_t &value) const noexcept {
    return impl::getAsUnsignedInteger(value, buffer_, size_);
}


ErrorCode Value::getAsInteger(std::int8_t &value) const noexcept {
    return impl::getAsSignedIntegerFromBigger(value, buffer_, size_);
}


ErrorCode Value::getAsInteger(std::int16_t &value) const noexcept {
    return impl::getAsSignedIntegerFromBigger(value, buffer_, size_);
}


ErrorCode Value::getAsInteger(std::int32_t &value) const noexcept {
    #if DBOR_HAS_FAST_64BIT_ARITH
        return impl::getAsSignedIntegerFromBigger(value, buffer_, size_);
    #else
        return impl::getAsSignedInteger(value, buffer_, size_);
    #endif
}


ErrorCode Value::getAsInteger(std::int64_t &value) const noexcept {
    return impl::getAsSignedInteger(value, buffer_, size_);
}
