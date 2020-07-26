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

namespace dbor::impl {
    #include "Dbor/Value.cpp.inc"  // function templates and static functions
}

using namespace dbor;


/**
 * Assigns an incomplete value of zero size in an empty buffer.
 * All \c get() methods will fail with ResultCode::INCOMPLETE.
 */
Value::Value() noexcept
    : buffer_(nullptr)
    , size_(0)
    , isComplete_(false)
{
}


/**
 * Assigns the first value in the non-zero buffer, without owning it, or
 * an incomplete value of zero size if the buffer is empty.
 *
 * If the buffer is not empty and \c buffer[0] .. \c buffer[n - 1] is a complete (well-formed or
 * ill-formed) DBOR value with n <= \c capacity, \c size() will be the size of this value
 * and \c isComplete() will be \c true.
 *
 * If the buffer is not empty and \c buffer[0] .. \c buffer[capacity - 1] does not starts with
 * a complete (well-formed or ill-formed) DBOR value, \c size() will be \c capacity
 * and \c isComplete() will be \c false.
 *
 * If the buffer is empty, \c size() will 0 and \c isComplete() will be \c false.
 *
 * The constructed instance does not own the buffer. As long as the instance exists, the assigned
 * buffer must not change.
 */
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


/**
 * \copybrief get(std::int64_t &) const
 * \sa get(std::int64_t &) const
 */
ResultCode Value::get(std::uint8_t &value) const noexcept {
    return impl::getUnsignedInteger(value, buffer_, size_);
}


/**
 * \copybrief get(std::int64_t &) const
 * \sa get(std::int64_t &) const
 */
ResultCode Value::get(std::uint16_t &value) const noexcept {
    return impl::getUnsignedInteger(value, buffer_, size_);
}


/**
 * \copybrief get(std::int64_t &) const
 * \sa get(std::int64_t &) const
 */
ResultCode Value::get(std::uint32_t &value) const noexcept {
    return impl::getUnsignedInteger(value, buffer_, size_);
}


/**
 * \copybrief get(std::int64_t &) const
 * \sa get(std::int64_t &) const
 */
ResultCode Value::get(std::uint64_t &value) const noexcept {
    return impl::getUnsignedInteger(value, buffer_, size_);
}


/**
 * \copybrief get(std::int64_t &) const
 * \sa get(std::int64_t &) const
 */
ResultCode Value::get(std::int8_t &value) const noexcept {
    return impl::getSignedInteger(value, buffer_, size_);
}


/**
 * \copybrief get(std::int64_t &) const
 * \sa get(std::int64_t &) const
 */
ResultCode Value::get(std::int16_t &value) const noexcept {
    return impl::getSignedInteger(value, buffer_, size_);
}


/**
 * \copybrief get(std::int64_t &) const
 * \sa get(std::int64_t &) const
 */
ResultCode Value::get(std::int32_t &value) const noexcept {
    return impl::getSignedInteger(value, buffer_, size_);
}


/**
 * \brief Decodes the first complete DBOR value in the assigned buffer into @c value.
 *
 * If the assigned buffer is non-empty and starts with a complete DBOR value:
 *
 *   DBOR value (complete)                                      | Return value                  | @c value
 *   -----------------------------------------------------------|-------------------------------|--------
 *   IntegerValue(\f$v\f$) with \f$v_{min} \le v \le v_{max}\f$ | ResultCode::OK                | \f$v\f$
 *   MinusZeroValue()                                           | ResultCode::APPROX_IMPRECISE  | 0
 *   IntegerValue(\f$v\f$) with \f$v < v_{min}\f$               | ResultCode::APPROX_EXTREME    | \f$v_{min}\f$
 *   IntegerValue(\f$v\f$) with \f$v > v_{max}\f$               | ResultCode::APPROX_EXTREME    | \f$v_{max}\f$
 *   MinusInfinityValue()                                       | ResultCode::APPROX_EXTREME    | \f$v_{min}\f$
 *   InfinityValue()                                            | ResultCode::APPROX_EXTREME    | \f$v_{max}\f$
 *   NoneValue()                                                | ResultCode::NO_OBJECT         | 0
 *   other                                                      | ResultCode::INCOMPATIBLE      | 0
 *
 * Otherwise: Returns ResultCode::INCOMPLETE and @c value = 0.
 *
 * \f$v_{min}\f$, \f$v_{max}\f$: Minimum, maximum value representable by @c value,
 */
ResultCode Value::get(std::int64_t &value) const noexcept {
    return impl::getSignedInteger(value, buffer_, size_);
}


/**
 * \brief Decodes the first complete DBOR value in the assigned buffer into @c value.
 *
 * If the assigned buffer is non-empty and starts with a complete DBOR value:
 *
 *   DBOR value (complete)                                                          | Return value                  | @c value
 *   -------------------------------------------------------------------------------|-------------------------------|----------
 *   MinusInfinityValue()                                                           | ResultCode::OK                | \f$-\infty\f$
 *   IntegerValue(0)                                                                | ResultCode::OK                | 0
 *   MinusZeroValue()                                                               | ResultCode::OK                | -0
 *   BinaryRationalValue, representing \f$v\f$ with \f$v_{min} \le v \le v_{max}\f$ | ResultCode::OK                | \f$v\f$
 *   InfinityValue()                                                                | ResultCode::OK                | \f$\infty\f$
 *   BinaryRationalValue, representing \f$v\f$ with \f$v_{min} \le v \le v_{max}\f$ | ResultCode::APPROX_IMPRECISE  | \f$v\f$ rounded towards 0
 *   BinaryRationalValue, representing \f$v\f$ with \f$v < v_{min}\f$               | ResultCode::APPROX_EXTREME    | \f$-\infty\f$
 *   BinaryRationalValue, representing \f$v\f$ with \f$v > v_{max}\f$               | ResultCode::APPROX_EXTREME    | \f$\infty\f$
 *   BinaryRationalValue, illformed                                                 | ResultCode::ILLFORMED         | NaN
 *   NoneValue()                                                                    | ResultCode::NO_OBJECT         | NaN
 *   other                                                                          | ResultCode::INCOMPATIBLE      | NaN
 *
 * Otherwise: Returns ResultCode::INCOMPLETE and @c value = NaN.
 *
 * \f$v_{min}\f$, \f$v_{max}\f$: Minimum, maximum finite value representable by @c value,
 */
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


/**
 * \copybrief get(float &) const
 * \sa get(float &) const
 */
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


/**
 * \brief Decodes the first complete DBOR value in the assigned buffer into @c mant and @c exp10,
 * representing the number \f$\text{mant} \cdot 10^{\text{exp10}}\f$.
 *
 * If the assigned buffer is non-empty and starts with a complete DBOR value:
 *
 *   DBOR value (complete)                                                                            | Return value                  | @c mant       | @c exp10
 *   -------------------------------------------------------------------------------------------------|-------------------------------|---------------|---------------
 *   IntegerValue(\f$m\f$) with \f$m_{min} \le m \le m_{max}\f$                                       | ResultCode::OK                | \f$m\f$       | 0
 *   DecimalRationalValue(m, e) with \f$m_{min} \le v \le m_{max}\f$, \f$e_{min} \le e \le e_{max}\f$ | ResultCode::OK                | \f$m\f$       | \f$e\f$
 *   MinusZeroValue()                                                                                 | ResultCode::APPROX_IMPRECISE  | 0             | 0
 *   IntegerValue(\f$m\f$) with \f$m < m_{min}\f$                                                     | ResultCode::APPROX_EXTREME    | \f$m_{min}\f$ | 0
 *   IntegerValue(\f$m\f$) with \f$m > m_{max}\f$                                                     | ResultCode::APPROX_EXTREME    | \f$m_{max}\f$ | 0
 *   DecimalRationalValue(\f$m, e\f$) with \f$m < m_{min}\f$, \f$e_{min} \le e \le e_{max}\f$         | ResultCode::APPROX_EXTREME    | \f$m_{min}\f$ | \f$e\f$
 *   DecimalRationalValue(\f$m, e\f$) with \f$m > m_{max}\f$, \f$e_{min} \le e \le e_{max}\f$         | ResultCode::APPROX_EXTREME    | \f$m_{max}\f$ | \f$e\f$
 *   MinusInfinityValue()                                                                             | ResultCode::APPROX_EXTREME    | \f$m_{min}\f$ | \f$e_{max}\f$
 *   InfinityValue()                                                                                  | ResultCode::APPROX_EXTREME    | \f$m_{max}\f$ | \f$e_{max}\f$
 *   DecimalRationalValue(\f$m, e\f$) with \f$e < e_{min}\f$ or \f$e > e_{max}\f$                     | ResultCode::UNSUPPORTED       | 0             | 0
 *   DecimalRationalValue(\f$m, e\f$), ill-formed                                                     | ResultCode::ILLFORMED         | 0             | 0
 *   NoneValue()                                                                                      | ResultCode::NO_OBJECT         | 0             | 0
 *   other                                                                                            | ResultCode::INCOMPATIBLE      | 0             | 0
 *
 * Otherwise: Returns ResultCode::INCOMPLETE and @c mant = @c exp10 = 0.
 *
 * \f$m_{min}\f$, \f$m_{max}\f$: Minimum, maximum finite value representable by @c mant,
 * \f$e_{min}\f$, \f$e_{max}\f$: Minimum, maximum finite value representable by @c exp10,
 */
ResultCode Value::get(std::int32_t &mant, std::int32_t &exp10) const noexcept {
    mant = 0;
    exp10 = 0;

    // if ResultCode::APPROX_IMPRECISE: mant > 0 if too large, mant < 0 if too small

    if (!isComplete_)
        return ResultCode::INCOMPLETE;

    if (buffer_[0] < 0x40) {
         // IntegerValue(mant), treat like DecimalRationalValue(mant, 0)
        ResultCode r = get(mant);
        // for ResultCode::APPROX_IMPRECISE: best approximation with given exp10
        return r == ResultCode::APPROX_EXTREME ? ResultCode::APPROX_IMPRECISE : r;
    }

    if (buffer_[0] < 0xD0)
        return ResultCode::INCOMPATIBLE;

    if (buffer_[0] >= 0xF0) {
        switch (buffer_[0]) {
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_ZERO):
            return ResultCode::APPROX_IMPRECISE;
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::MINUS_INF):
            mant = std::numeric_limits<std::int32_t>::min();
            exp10 = std::numeric_limits<std::int32_t>::max();
            return ResultCode::APPROX_EXTREME;
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::INF):
            mant = std::numeric_limits<std::int32_t>::max();
            exp10 = std::numeric_limits<std::int32_t>::max();
            return ResultCode::APPROX_EXTREME;
        case static_cast<std::uint8_t>(Encoding::SingleByteValue::NONE):
            return ResultCode::NO_OBJECT;
        default:
            return ResultCode::INCOMPATIBLE;
        }
    }

    // PowerOfTenToken(e)

    std::uint32_t eAbs;
    std::size_t firstTokenSize = 0;

    if ((buffer_[0] & 0xF0) == 0xE0) {
        // |e| <= 8
        eAbs = (buffer_[0] & 7u) + 1;
        firstTokenSize = 1;
    } else if ((buffer_[0] & 0xF0) == 0xD0) {
        // |e| > 8
        firstTokenSize = 2u + (buffer_[0] & 7u);
        if (size_ < firstTokenSize)
            return ResultCode::INCOMPLETE;
        if (!Encoding::decodeNaturalTokenData(eAbs, &buffer_[1], firstTokenSize - 1u, 8u))
            eAbs = std::numeric_limits<std::uint32_t>::max();
    }

    if (size_ <= firstTokenSize || buffer_[firstTokenSize] == 0
        || buffer_[firstTokenSize] >= 0x40)
        return ResultCode::ILLFORMED;  // not followed by an IntegerToken(v) with v != 0

    std::int32_t m;
    ResultCode r = impl::getSignedInteger(m, &buffer_[firstTokenSize], size_ - firstTokenSize);

    if (buffer_[0] & 8u) {
        // exp10 < 0
        if (eAbs <= -static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::min())) {
            mant = m;  // best approximation for given exp10
            exp10 = -static_cast<std::int32_t>(eAbs);
            if (r == ResultCode::APPROX_EXTREME)
                r = ResultCode::APPROX_IMPRECISE;
        } else
            r = ResultCode::UNSUPPORTED;
    } else {
        // exp10 > 0
        if (eAbs <= static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max())) {
            mant = m;
            exp10 = static_cast<std::int32_t>(eAbs);
        } else
            r = ResultCode::UNSUPPORTED;
    }

    return r;
}


/**
 * \brief Decodes the first complete DBOR value in the assigned buffer into @c bytes and
 * @c stringSize, representing a bytes string @c bytes[0] .. @c bytes[stringSize - 1].
 *
 * If the assigned buffer is non-empty and starts with a complete DBOR value:
 *
 *   DBOR value (complete)                   | Return value                  | @c bytes            | @c stringSize
 *   ----------------------------------------|-------------------------------|---------------------|---------------
 *   ByteStringValue(\f$b_1, \ldots, b_m\f$) | ResultCode::OK                | pointer to \$b_1\$  | \f$m\f$
 *   NoneValue()                             | ResultCode::NO_OBJECT         | @c nullptr          | 0
 *   other                                   | ResultCode::INCOMPATIBLE      | @c nullptr          | 0
 *
 * Otherwise: Returns ResultCode::INCOMPLETE and @c mant = @c nullptr, @c stringSize = 0.
 */
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


/**
 * \brief Decodes the first complete DBOR value in the assigned buffer into @c value,
 * representing an Unicode string of at most @c maxSize byte in UTF-8 encoding.
 *
 * If the assigned buffer is non-empty and starts with a complete DBOR value:
 *
 *   DBOR value (complete)                                                   | Return value             | @c value
 *   ------------------------------------------------------------------------|--------------------------|----------------------------
 *   Utf8StringValue(\f$b_1, \ldots, b_m\f$) with \f$m \le \text{maxSize}\f$ | ResultCode::OK           | String(&\f$b_1\f$, \f$m\f$)
 *   Utf8StringValue(\f$b_1, \ldots, b_m\f$) with \f$m > \text{maxSize}\f$   | ResultCode::RANGE        | String()
 *   NoneValue()                                                             | ResultCode::NO_OBJECT    | String()
 *   other                                                                   | ResultCode::INCOMPATIBLE | String()
 *
 * Otherwise: Returns ResultCode::INCOMPLETE and @c value = String().
 *
 * Note:
 * ResultCode::OK does not mean that this value is a well-formed Utf8StringValue.
 * Use \link String::check() value.check() \endlink or
 * \link String::getUtf8() value.getXXX() \endlink in addition.
 */
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
    if (stringSize > maxSize)
        return ResultCode::RANGE;

    value = String(&buffer_[sizeOfFirstToken], stringSize);
    return ResultCode::OK;
}


int Value::compareTo(const Value &other) const noexcept {
    std::size_t n = size_;

    if (!n || !other.size_)
        return int(n > 0) - int(other.size_ > 0);

    if (buffer_[0] < other.buffer_[0])
        return -1;
    if (buffer_[0] > other.buffer_[0])
        return 1;

    if (n < other.size_)
        return -1;
    if (n > other.size_)
        return 1;

    for (std::size_t i = n - 1u; i > 1; i--) {
        if (buffer_[i] < other.buffer_[i])
            return -1;
        if (buffer_[i] > other.buffer_[i])
            return 1;
    }

    if (isComplete_ == other.isComplete_)
        return 0;
    return isComplete_ ? 1 : -1;
}
