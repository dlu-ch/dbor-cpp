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


ResultCode Value::get(std::uint8_t &value) const noexcept {
    return impl::getUnsignedInteger(value, buffer_, size_);
}


ResultCode Value::get(std::uint16_t &value) const noexcept {
    return impl::getUnsignedInteger(value, buffer_, size_);
}


ResultCode Value::get(std::uint32_t &value) const noexcept {
    return impl::getUnsignedInteger(value, buffer_, size_);
}


ResultCode Value::get(std::uint64_t &value) const noexcept {
    return impl::getUnsignedInteger(value, buffer_, size_);
}


ResultCode Value::get(std::int8_t &value) const noexcept {
    return impl::getSignedInteger(value, buffer_, size_);
}


ResultCode Value::get(std::int16_t &value) const noexcept {
    return impl::getSignedInteger(value, buffer_, size_);
}


ResultCode Value::get(std::int32_t &value) const noexcept {
    return impl::getSignedInteger(value, buffer_, size_);
}


ResultCode Value::get(std::int64_t &value) const noexcept {
    return impl::getSignedInteger(value, buffer_, size_);
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


ResultCode Value::get(std::int32_t &mant, std::int32_t &exp10) const noexcept {
    mant = 0;
    exp10 = 0;

    // if ResultCode::APPROX_IMPRECISE: mant > 0 if too large, mant < 0 if too small

    if (!isComplete_)
        return ResultCode::INCOMPLETE;

    if (buffer_[0] < 0x40)
         // IntegerValue(mant), treat like DecimalRationalValue(mant, 0)
        return get(mant);

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
        if (eAbs > -static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::min())) {
            mant = 0;  // TODO is there an efficient way to approximate?
            exp10 = std::numeric_limits<std::int32_t>::min();
            r = ResultCode::APPROX_IMPRECISE;
        } else {
            mant = m;
            exp10 = -static_cast<std::int32_t>(eAbs);
            if (r == ResultCode::APPROX_EXTREME)
                r = ResultCode::APPROX_IMPRECISE;  // TODO is there an efficient way to approximate?
        }
    } else {
        // exp10 > 0
        if (eAbs > static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max())) {
            mant =
                m > 0 ?
                std::numeric_limits<std::int32_t>::max() :
                std::numeric_limits<std::int32_t>::min();
            exp10 = std::numeric_limits<std::int32_t>::max();
            r = ResultCode::APPROX_EXTREME;
        } else {
            mant = m;
            exp10 = static_cast<std::int32_t>(eAbs);
        }
    }

    return r;
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
