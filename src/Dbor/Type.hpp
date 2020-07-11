// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_TYPE_HPP_
#define DBOR_TYPE_HPP_

#include <cstdint>
#include <type_traits>

namespace dbor {

    // Result code       Returned value
    // --------------    ----------------------------------------------
    //
    // OK                object (exactly)
    //
    // APPROX_PRECISION  nearest representable but different from object
    //                   (for NumberValue: rounded towards 0)
    // APPROX_RANGE      nearest representable but different from object
    //
    // NO_OBJECT         default (0 or NaN)
    // INCOMPATIBLE      default (0 or NaN)
    // ILLFORMED         default (0 or NaN)
    // INCOMPLETE        default (0 or NaN)
    //
    enum class ResultCodes: std::uint_fast8_t {
        OK = 0u,

        APPROX_PRECISION = 1u << 0u,
        APPROX_RANGE     = 1u << 1u,

        NO_OBJECT    = 1u << 2u,
        INCOMPATIBLE = 1u << 3u,
        ILLFORMED    = 1u << 4u,
        INCOMPLETE   = 1u << 5u,
    };


    // Beware: order of evaluation is unspecified for function calls and most operators.
    // Exceptions: <<, >>, ||, &&, comma, compound assignment operators (|= etc.), [], ->, *

    // Set union: return all errors in included in left or in right.
    // Every value computation and side-effect of left is sequenced before right.
    inline ResultCodes operator<<(ResultCodes left, ResultCodes right) noexcept {
        typedef std::underlying_type<ResultCodes>::type U;
        return static_cast<ResultCodes>(static_cast<U>(left) | static_cast<U>(right));
    }

    // Set union: include in left all errors included in right.
    // Every value computation and side-effect of left is sequenced before right.
    inline ResultCodes &operator<<=(ResultCodes &left, ResultCodes right) noexcept {
        typedef std::underlying_type<ResultCodes>::type U;
        left = static_cast<ResultCodes>(static_cast<U>(left) | static_cast<U>(right));
        return left;
    }

    // Set intersection: exclude in left all errors not included in right.
    // Exclude in left all bits cleared in right.
    // Every value computation and side-effect of left is sequenced before right.
    inline ResultCodes operator&=(ResultCodes &left, ResultCodes right) noexcept {
        typedef std::underlying_type<ResultCodes>::type U;
        left = static_cast<ResultCodes>(static_cast<U>(left) & static_cast<U>(right));
        return left;
    }

    // Symmetric set difference: exclude in left all errors included in right.
    // Exclude in left all bits set in right.
    // Every value computation and side-effect of left is sequenced before right.
    inline ResultCodes &operator-=(ResultCodes &left, ResultCodes right) noexcept {
        typedef std::underlying_type<ResultCodes>::type U;
        left = static_cast<ResultCodes>(static_cast<U>(left) & ~static_cast<U>(right));
        return left;
    }

    // = OK?
    inline bool isOk(ResultCodes errorCode) noexcept {
        return errorCode == ResultCodes::OK;
    }

    // = OK after all bits in exceptions are cleared?
    inline bool isOkExcept(ResultCodes errorCode, ResultCodes exceptions) noexcept {
        typedef std::underlying_type<ResultCodes>::type U;
        return isOk(static_cast<ResultCodes>(static_cast<U>(errorCode)
                  & ~static_cast<U>(exceptions)));
    }

    // = APPROX_PRECISION or = APPROX_RANGE?
    inline bool isApprox(ResultCodes errorCode) noexcept {
        return errorCode >= ResultCodes::APPROX_PRECISION && errorCode <= ResultCodes::APPROX_RANGE;
    }

};


#endif  // DBOR_TYPE_HPP_
