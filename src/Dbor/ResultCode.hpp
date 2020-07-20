// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_RESULTCODE_HPP_
#define DBOR_RESULTCODE_HPP_

#include <cstdint>
#include <type_traits>

namespace dbor {

    /**
     * \brief Code of the result of an operation involving a DBOR value.
     *
     * Result codes can be combined to \c ResultCodeSet with the << operator.
     *
     * Example:
     * \code
     * ResultCode::NO_OBJECT << ResultCode::INCOMPATIBLE
     * \endcode
     * is a \c ResultCodeSet with the members ResultCode::NO_OBJECT and ResultCode::INCOMPATIBLE.
     */
    enum class ResultCode: std::uint_fast8_t {  // the higher the value the more severe
        OK               = 0u,

        APPROX_IMPRECISE = 1u << 0u,
        APPROX_EXTREME   = 1u << 1u,

        RANGE            = 1u << 2u,
        NO_OBJECT        = 1u << 3u,
        INCOMPATIBLE     = 1u << 4u,
        UNSUPPORTED      = 1u << 5u,
        ILLFORMED        = 1u << 6u,
        INCOMPLETE       = 1u << 7u
    };

    /**
     * \brief Set of results from \c ResultCode other than ResultCode::OK.
     * Use leastSevereIn() to iterate over the members of the set.
     */
    enum class ResultCodeSet: std::underlying_type<ResultCode>::type {
        NONE = 0u,             ///< set of "not ok" results is empty
        ALL = (1u << 8u) - 1u  ///< set of all "not ok" results
    };


    // Beware: order of evaluation is unspecified for function calls and most operators.
    // Exceptions: <<, >>, ||, &&, comma, compound assignment operators and some others.
    //
    // For the following operators, every value computation and side-effect of left
    // is sequenced before right.

    constexpr ResultCodeSet operator<<(ResultCode left, ResultCode right) noexcept;
    constexpr ResultCodeSet operator<<(ResultCodeSet left, ResultCode right) noexcept;
    constexpr ResultCodeSet operator<<(ResultCode left, ResultCodeSet right) noexcept;
    constexpr ResultCodeSet operator<<(ResultCodeSet left, ResultCodeSet right) noexcept;

    ResultCodeSet &operator<<=(ResultCodeSet &left, ResultCode right) noexcept;
    ResultCodeSet &operator<<=(ResultCodeSet &left, ResultCodeSet right) noexcept;

    ResultCodeSet operator&=(ResultCodeSet &left, ResultCode right) noexcept;
    ResultCodeSet operator&=(ResultCodeSet &left, ResultCodeSet right) noexcept;

    ResultCodeSet &operator-=(ResultCodeSet &left, ResultCode right) noexcept;
    ResultCodeSet &operator-=(ResultCodeSet &left, ResultCodeSet right) noexcept;

    // = ResultCode::OK?
    constexpr bool isOk(ResultCode resultCode) noexcept;
    constexpr bool isOk(ResultCodeSet resultCodes) noexcept;

    // = ResultCode::OK after all bits in exceptions are cleared?
    constexpr bool isOkExcept(ResultCode resultCode, ResultCode exception) noexcept;
    constexpr bool isOkExcept(ResultCodeSet resultCodes, ResultCode exception) noexcept;
    constexpr bool isOkExcept(ResultCode resultCode, ResultCodeSet exceptions) noexcept;
    constexpr bool isOkExcept(ResultCodeSet resultCodes, ResultCodeSet exceptions) noexcept;

    // = ResultCode::APPROX_IMPRECISE or = ResultCode::APPROX_EXTREME?
    constexpr bool isApprox(ResultCode resultCode) noexcept;
    constexpr bool isApprox(ResultCodeSet resultCodes) noexcept;

    constexpr ResultCode leastSevereIn(ResultCodeSet resultCodes) noexcept;

}


// Inline implementations ---

namespace dbor {

    /** \brief Set union: return all "not ok" results included in left or in right. */
    inline constexpr ResultCodeSet operator<<(ResultCodeSet left, ResultCodeSet right) noexcept {
        typedef std::underlying_type<ResultCode>::type U;
        return static_cast<ResultCodeSet>(static_cast<U>(left) | static_cast<U>(right));
    }

    /** \copydoc operator<<(ResultCodeSet, ResultCodeSet) */
    inline constexpr ResultCodeSet operator<<(ResultCode left, ResultCode right) noexcept {
        return static_cast<ResultCodeSet>(left) << static_cast<ResultCodeSet>(right);
    }

    /** \copydoc operator<<(ResultCodeSet, ResultCodeSet) */
    inline constexpr ResultCodeSet operator<<(ResultCodeSet left, ResultCode right) noexcept {
        return left << static_cast<ResultCodeSet>(right);
    }

    /** \copydoc operator<<(ResultCodeSet, ResultCodeSet) */
    inline constexpr ResultCodeSet operator<<(ResultCode left, ResultCodeSet right) noexcept {
        return static_cast<ResultCodeSet>(left) << right;
    }

    /** \brief Set union: include in left all "not ok" results included in right. */
    inline ResultCodeSet &operator<<=(ResultCodeSet &left, ResultCode right) noexcept {
        left = left << right;
        return left;
    }

    /** \copydoc operator<<=(ResultCodeSet &, ResultCode) */
    inline ResultCodeSet &operator<<=(ResultCodeSet &left, ResultCodeSet right) noexcept {
        left = left << right;
        return left;
    }

    /** \brief Set intersection: exclude in left all "not ok" results not included in right. */
    inline ResultCodeSet operator&=(ResultCodeSet &left, ResultCodeSet right) noexcept {
        typedef std::underlying_type<ResultCode>::type U;
        left = static_cast<ResultCodeSet>(static_cast<U>(left) & static_cast<U>(right));
        return left;
    }

    /** \copydoc operator&=(ResultCodeSet &, ResultCodeSet) */
    inline ResultCodeSet operator&=(ResultCodeSet &left, ResultCode right) noexcept {
        left &= static_cast<ResultCodeSet>(right);
        return left;
    }

    /** \brief Symmetric set difference: exclude in left all "not ok" results included in right. */
    inline ResultCodeSet &operator-=(ResultCodeSet &left, ResultCodeSet right) noexcept {
        typedef std::underlying_type<ResultCode>::type U;
        left = static_cast<ResultCodeSet>(static_cast<U>(left) & ~static_cast<U>(right));
        return left;
    }

    /** \copydoc operator-=(ResultCodeSet &, ResultCodeSet) */
    inline ResultCodeSet &operator-=(ResultCodeSet &left, ResultCode right) noexcept {
        left -= static_cast<ResultCodeSet>(right);
        return left;
    }

    inline constexpr bool isOk(ResultCode resultCode) noexcept {
        return resultCode == ResultCode::OK;
    }

    inline constexpr bool isOk(ResultCodeSet resultCodes) noexcept {
        return resultCodes == ResultCodeSet::NONE;
    }

    inline constexpr bool isOkExcept(ResultCodeSet resultCodes, ResultCodeSet exceptions) noexcept {
        typedef std::underlying_type<ResultCode>::type U;
        return isOk(static_cast<ResultCode>(static_cast<U>(resultCodes)
                  & ~static_cast<U>(exceptions)));
    }

    inline constexpr bool isOkExcept(ResultCodeSet resultCodes, ResultCode exception) noexcept {
        return isOkExcept(resultCodes, static_cast<ResultCodeSet>(exception));
    }

    inline constexpr bool isOkExcept(ResultCode resultCode, ResultCodeSet exceptions) noexcept {
        return isOkExcept(static_cast<ResultCodeSet>(resultCode), exceptions);
    }

    inline constexpr bool isOkExcept(ResultCode resultCode, ResultCode exception) noexcept {
        return isOkExcept(static_cast<ResultCodeSet>(resultCode),
                          static_cast<ResultCodeSet>(exception));
    }

    inline constexpr bool isApprox(ResultCodeSet resultCodes) noexcept {
        typedef std::underlying_type<ResultCode>::type U;
        return resultCodes != ResultCodeSet::NONE
           && (static_cast<U>(resultCodes)
               & ~static_cast<U>(ResultCode::APPROX_IMPRECISE << ResultCode::APPROX_EXTREME)) == 0;
    }

    inline constexpr bool isApprox(ResultCode resultCode) noexcept {
        return isApprox(static_cast<ResultCodeSet>(resultCode));
    }

    /**
     * \brief Of all "not ok" results in resultCodes that are included in ResultCode::ALL,
     * return theone with the lowest value.
     * Use this to iterate over "not ok" results.
     */
    inline constexpr ResultCode leastSevereIn(ResultCodeSet resultCodes) noexcept {
        typedef std::underlying_type<ResultCode>::type U;
        return static_cast<ResultCode>(static_cast<U>(resultCodes) & -static_cast<U>(resultCodes));
    }

}

#endif  // DBOR_RESULTCODE_HPP_
