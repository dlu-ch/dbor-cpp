// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_RESULTCODES_HPP_
#define DBOR_RESULTCODES_HPP_

#include <cstdint>
#include <type_traits>

namespace dbor {

    // Result code       Result object (e.g. decoded value in target type)
    // --------------    ---------------------------------------------------------------------------
    //
    // OK                object (exactly)
    //
    // APPROX_IMPRECISE  representable approximation of object (for NumberValue: rounded towards 0)
    // APPROX_EXTREME    minimum or maximum of representable objects because object outside
    //
    // RANGE             -
    // NO_OBJECT         -
    // INCOMPATIBLE      -
    // ILLFORMED         -
    // INCOMPLETE        -
    //
    enum class ResultCodes: std::uint_fast8_t {  // the higher the value the more severe
        OK = 0u,  // (set of "not ok" results is empty)

        APPROX_IMPRECISE = 1u << 0u,
        APPROX_EXTREME   = 1u << 1u,

        RANGE        = 1u << 2u,
        NO_OBJECT    = 1u << 3u,
        INCOMPATIBLE = 1u << 4u,
        ILLFORMED    = 1u << 5u,
        INCOMPLETE   = 1u << 6u,

        ALL = (INCOMPLETE << 1u) - 1u  // (set of all "not ok" results)
    };


    // Beware: order of evaluation is unspecified for function calls and most operators.
    // Exceptions: <<, >>, ||, &&, comma, compound assignment operators ad some others.
    //
    // For the following operators, every value computation and side-effect of left
    // is sequenced before right.

    // Set union: return all "not ok" results included in left or in right.
    ResultCodes operator<<(ResultCodes left, ResultCodes right) noexcept;

    // Set union: include in left all "not ok" results included in right.
    ResultCodes &operator<<=(ResultCodes &left, ResultCodes right) noexcept;

    // Set intersection: exclude in left all "not ok" results not included in right.
    ResultCodes operator&=(ResultCodes &left, ResultCodes right) noexcept;

    // Symmetric set difference: exclude in left all "not ok" results included in right.
    ResultCodes &operator-=(ResultCodes &left, ResultCodes right) noexcept;

    // Of all "not ok" results in resultCodes that are included in ResultCodes::ALL, return the
    // one with the lowest value.
    // Use this to iterate over "not ok" results.
    ResultCodes leastSevereIn(ResultCodes resultCodes) noexcept;

    // = ResultCodes::OK?
    bool isOk(ResultCodes resultCode) noexcept;

    // = ResultCodes::OK after all bits in exceptions are cleared?
    bool isOkExcept(ResultCodes resultCode, ResultCodes exceptions) noexcept;

    // = ResultCodes::APPROX_IMPRECISE or = ResultCodes::APPROX_EXTREME?
    bool isApprox(ResultCodes resultCode) noexcept;

}


// Inline implementations ---

namespace dbor {

    inline ResultCodes operator<<(ResultCodes left, ResultCodes right) noexcept {
        typedef std::underlying_type<ResultCodes>::type U;
        return static_cast<ResultCodes>(static_cast<U>(left) | static_cast<U>(right));
    }

    inline ResultCodes &operator<<=(ResultCodes &left, ResultCodes right) noexcept {
        typedef std::underlying_type<ResultCodes>::type U;
        left = static_cast<ResultCodes>(static_cast<U>(left) | static_cast<U>(right));
        return left;
    }

    inline ResultCodes operator&=(ResultCodes &left, ResultCodes right) noexcept {
        typedef std::underlying_type<ResultCodes>::type U;
        left = static_cast<ResultCodes>(static_cast<U>(left) & static_cast<U>(right));
        return left;
    }

    inline ResultCodes &operator-=(ResultCodes &left, ResultCodes right) noexcept {
        typedef std::underlying_type<ResultCodes>::type U;
        left = static_cast<ResultCodes>(static_cast<U>(left) & ~static_cast<U>(right));
        return left;
    }

    inline ResultCodes leastSevereIn(ResultCodes resultCodes) noexcept {
        typedef std::underlying_type<ResultCodes>::type U;
        U u = static_cast<U>(resultCodes);
        return static_cast<ResultCodes>(u & -u);
    }

    inline bool isOk(ResultCodes resultCode) noexcept {
        return resultCode == ResultCodes::OK;
    }

    inline bool isOkExcept(ResultCodes resultCode, ResultCodes exceptions) noexcept {
        typedef std::underlying_type<ResultCodes>::type U;
        return isOk(static_cast<ResultCodes>(static_cast<U>(resultCode)
                  & ~static_cast<U>(exceptions)));
    }

    inline bool isApprox(ResultCodes resultCode) noexcept {
        return resultCode >= ResultCodes::APPROX_IMPRECISE
            && resultCode <= ResultCodes::APPROX_EXTREME;
    }

}

#endif  // DBOR_RESULTCODES_HPP_
