// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_VALUE_HPP_
#define DBOR_VALUE_HPP_

#include "Dbor/ResultCode.hpp"
#include "Dbor/String.hpp"

namespace dbor {

    /**
     * \brief DBOR value (well-formed, ill-formed or incomplete) in a non-empty immutable
     * byte buffer or an incomplete value of zero size in an empty buffer.
     *
     * Such a value can be \em complete or \em incomplete.
     * It is complete if and only if the assigned buffer is long enough to determine type and
     * size of the value and has the determined size of the value.
     * A complete value can be \em well-formed or \em ill-formed.
     *
     * Example:
     * \code
     * std::uint8_t buffer[] = {
     *     0xC8, 0x00,  // BinaryRationalValue representing 0.125
     *     0x07         // IntegerValue(7)
     * };
     * dbor::Value value(buffer, sizeof(buffer));
     *     // value.buffer() is buffer
     *     // value.size() is 2
     *     // value.isComplete() is true
     *
     * dbor::Value incompleteValue(buffer, 1u);
     *     // value.buffer() is buffer
     *     // value.size() is 1
     *     // value.isComplete() is false
     * \endcode
     *
     * The \c get(...) methods decode the object represented by the assigned buffer into
     * ..., if possible. The result code returned by any of the \c get() methods has the following
     * meaning with respect to ...:
     *
     *   Result code      | Result object represented by parameters ... of get(...)
     *   -----------------|-----------------------------------------------------------------------------------------------------------------
     *   OK               | same as object (exactly)
     *   APPROX_IMPRECISE | representable approximation of object inside range of representable objects (for NumberValue: rounded towards 0)
     *   APPROX_EXTREME   | minimum or maximum of representable objects because object outside
     *   RANGE            | default for target type (0, empty, NaN, ...)
     *   NO_OBJECT        | default for target type (0, empty, NaN, ...)
     *   INCOMPATIBLE     | default for target type (0, empty, NaN, ...)
     *   ILLFORMED        | default for target type (0, empty, NaN, ...)
     *   INCOMPLETE       | default for target type (0, empty, NaN, ...)
     *
     * Note that result codes can be combined with the << operator to a \c ResultCodeSet.
     *
     * Use \c ValueSequence to decode multiple values like this:
     * \code
     *   std::uint8_t a;
     *   dbor::String b;
     *   std::size_t bLength;
     *   float c;
     *
     *   dbor::ValueSequence values(...);
     *   auto iter = values.begin();  // iterates over values (*iter is a dbor::Value)
     *   dbor::ResultCodeSet results =
     *              iter->get(a)
     *       << (++iter)->get(b) << b.check(bLength, 0x0020, 0xFFFF)
     *       << (++iter)->get(c);
     *
     *   if (isOk(results))
     *       ...  // use a, b, c
     * \endcode
     */
    class Value {
    public:
        Value() noexcept;  // incomplete with size() = 0
        Value(const uint8_t *buffer, std::size_t capacity) noexcept;  // first in buffer
        Value(const Value &) noexcept = default;
        Value &operator=(const Value &) noexcept = default;

        const uint8_t *buffer() const noexcept;  // nullptr if and only if size() = 0
        std::size_t size() const noexcept;  // = 0 if and only if buffer() = nullptr
        bool isComplete() const noexcept; // true only if size() > 0

        bool isNone() const noexcept;
        bool isNumberlike() const noexcept;
        bool isNumber() const noexcept;  // well-formed or ill-formed/incomplete
        bool isString() const noexcept;  // well-formed or ill-formed/incomplete
        bool isContainer() const noexcept;  // well-formed or ill-formed/incomplete

        ResultCode get(std::uint8_t &value) const noexcept;
        ResultCode get(std::uint16_t &value) const noexcept;
        ResultCode get(std::uint32_t &value) const noexcept;
        ResultCode get(std::uint64_t &value) const noexcept;
        ResultCode get(std::int8_t &value) const noexcept;
        ResultCode get(std::int16_t &value) const noexcept;
        ResultCode get(std::int32_t &value) const noexcept;
        ResultCode get(std::int64_t &value) const noexcept;

        ResultCode get(float &value) const noexcept;
        ResultCode get(double &value) const noexcept;
        ResultCode get(std::int32_t &mant, std::int32_t &exp10) const noexcept;

        ResultCode get(const std::uint8_t *&bytes, std::size_t &size) const noexcept;
        ResultCode get(String &value, std::size_t maxSize) const noexcept;

        /**
         * \brief Returns 0 if this and \c other are equal, -1 if this is < \c other and 1
         * if this is > \c other when compared as (complete or incomplete) byte sequences.
         *
         * Of two otherwise equal values a, b with different isComplete(),
         * the one with \c isComplete() = \false is smaller.
         *
         * This defines a strict total order on the set of all (well-formed, ill-formed or
         * incomplete) DBOR value representable as Value instances, with \c Value() as its
         * least element.
         *
         * If this and \c other are IntegerValue(\f$a\f$) and IntegerValue(\f$b\f$):
         * Returns -1 if and only if \f$|a| < |b|\f$ and \f$a \cdot b \ge 0\f$.
         */
        int compareTo(const Value &other) const noexcept;

    protected:
        const uint8_t *buffer_;
        std::size_t size_;
        bool isComplete_;
    };

}


// Inline implementations ---

inline const uint8_t *dbor::Value::buffer() const noexcept {
    return buffer_;
}


inline std::size_t dbor::Value::size() const noexcept {
    return size_;
}


/** \brief Is a non-empty buffer assigned containing a complete value? */
inline bool dbor::Value::isComplete() const noexcept {
    return isComplete_;
}


#endif  // DBOR_VALUE_HPP_
