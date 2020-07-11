// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_TYPE_HPP_
#define DBOR_TYPE_HPP_

#include <cstdint>
#include <type_traits>

namespace dbor {

    // Error code      Returned value
    // --------------  ----------------------------------------------
    //
    // OK_PRECISE      object (exactly)
    //
    // OK_IMPRECISE    nearest representable, different from object
    //                 (for NumberValue: rounded towards 0)
    // OUT_OF_RANGE    nearest representable, different from object
    //
    // NO_OBJECT       default (0 or NaN)
    // INCOMPATIBLE    default (0 or NaN)
    // ILLFORMED       default (0 or NaN)
    // INCOMPLETE      default (0 or NaN)
    //
    enum class ErrorCodes: std::uint_fast8_t {
        OK_PRECISE   = 0u,

        OK_IMPRECISE = 1u << 0u,  // TODO rename with common prefix (without OK)
        OUT_OF_RANGE = 1u << 1u,  // TODO rename with common prefix

        NO_OBJECT    = 1u << 2u,
        INCOMPATIBLE = 1u << 3u,
        ILLFORMED    = 1u << 4u,
        INCOMPLETE   = 1u << 5u,
    };


    // Beware: order of evaluation is unspecified for function calls and most operators.
    // Exceptions: <<, >>, ||, &&, comma, compound assignment operators (|= etc.), [], ->, *

    // Set union: return all errors in included in left or in right.
    // Every value computation and side-effect of left is sequenced before right.
    inline ErrorCodes operator<<(ErrorCodes left, ErrorCodes right) noexcept {
        typedef std::underlying_type<ErrorCodes>::type U;
        return static_cast<ErrorCodes>(static_cast<U>(left) | static_cast<U>(right));
    }

    // Set union: include in left all errors included in right.
    // Every value computation and side-effect of left is sequenced before right.
    inline ErrorCodes &operator<<=(ErrorCodes &left, ErrorCodes right) noexcept {
        typedef std::underlying_type<ErrorCodes>::type U;
        left = static_cast<ErrorCodes>(static_cast<U>(left) | static_cast<U>(right));
        return left;
    }

    // Set intersection: exclude in left all errors not included in right.
    // Exclude in left all bits cleared in right.
    // Every value computation and side-effect of left is sequenced before right.
    inline ErrorCodes operator&=(ErrorCodes &left, ErrorCodes right) noexcept {
        typedef std::underlying_type<ErrorCodes>::type U;
        left = static_cast<ErrorCodes>(static_cast<U>(left) & static_cast<U>(right));
        return left;
    }

    // Symmetric set difference: exclude in left all errors included in right.
    // Exclude in left all bits set in right.
    // Every value computation and side-effect of left is sequenced before right.
    inline ErrorCodes &operator-=(ErrorCodes &left, ErrorCodes right) noexcept {
        typedef std::underlying_type<ErrorCodes>::type U;
        left = static_cast<ErrorCodes>(static_cast<U>(left) & ~static_cast<U>(right));
        return left;
    }

    // = OK_PRECISE?
    inline bool isOk(ErrorCodes errorCode) noexcept {
        return errorCode == ErrorCodes::OK_PRECISE;
    }

    // = OK_PRECISE after all bits in exceptions are cleared?
    inline bool isOkExcept(ErrorCodes errorCode, ErrorCodes exceptions) noexcept {
        typedef std::underlying_type<ErrorCodes>::type U;
        return isOk(static_cast<ErrorCodes>(static_cast<U>(errorCode)
                  & ~static_cast<U>(exceptions)));
    }

    // = OK_IMPRECISE or = OUT_OF_RANGE?
    inline bool isApprox(ErrorCodes errorCode) noexcept {
        return errorCode >= ErrorCodes::OK_IMPRECISE && errorCode <= ErrorCodes::OUT_OF_RANGE;
    }

};


#endif  // DBOR_TYPE_HPP_
