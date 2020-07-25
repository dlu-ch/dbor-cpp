// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_SIZEOF_HPP_
#define DBOR_SIZEOF_HPP_

#include <cstdint>
#include <limits>
#include <type_traits>
#include <initializer_list>

namespace dbor {

    /**
     * Compile-time size calculation.
     *
     * Usage example:
     * \code
     * std::uint8_t buffer[dbor::SizeOf::integer(127) + dbor::SizeOf::byteString(42)];
     * ...
     * \endcode
     */
    struct SizeOf {
        SizeOf() = delete;

        /** \brief Returns the size of a IntegerValue(value) in byte. */
        constexpr static std::size_t integer(unsigned long long int value) noexcept;
        /** \copydoc integer(unsigned long long int) */
        constexpr static std::size_t integer(unsigned long int value) noexcept;
        /** \copydoc integer(unsigned long long int) */
        constexpr static std::size_t integer(unsigned int value) noexcept;
        /** \copydoc integer(unsigned long long int) */
        constexpr static std::size_t integer(signed long long int value) noexcept;
        /** \copydoc integer(unsigned long long int) */
        constexpr static std::size_t integer(signed long int value) noexcept;
        /** \copydoc integer(unsigned long long int) */
        constexpr static std::size_t integer(signed int value) noexcept;

        /**
         * \brief  Returns the size of a ByteStringValue(<b1, ..., bn>) in byte,
         * where n is \c stringSize.
         */
        constexpr static std::size_t byteString(std::size_t stringSize) noexcept;
        /**
         * \brief  Returns the size of a ByteStringValue(<b1, ..., bn>) in byte,
         * where n is \c bytes.size().
         */
        constexpr static std::size_t byteString(std::initializer_list<std::uint8_t> bytes) noexcept;

        /**
         * \brief Returns the size of a Utf8StringValue(<b1, ..., bn>) in byte,
         * where n is \c stringSize.
         */
        constexpr static std::size_t utf8String(std::size_t stringSize) noexcept;

        /**
         * \brief  Returns the size of a Utf8StringValue(<b1, ..., bn>) in byte,
         * where n is the size of of \c p in byte if \c is not \c nullptr and 0 otherwise.
         */
        constexpr static std::size_t utf8String(const char *p) noexcept;

        /**
         * \brief Returns the size of the NUL-terminated string in \c p or
         * 0 if \c p = \c nullptr.
         */
        constexpr static std::size_t stringSizeOf(const char *p) noexcept;

        /** \brief Returns max(n + m, std::numeric_limits<std::size_t>::max()). */
        constexpr static std::size_t addSaturating(std::size_t n, std::size_t m) noexcept;

        template <typename T>
        struct Integer {
            Integer() = delete;
            static_assert(std::is_integral<T>::value, "");
            constexpr static std::size_t value = integer(std::numeric_limits<T>::max());
        };


        template <std::size_t first, std::size_t... others>
        struct Add {
            Add() = delete;
            static_assert(Add<others...>::value
                          <= std::numeric_limits<std::size_t>::max() - first,
                          "result not representable as std::size_t");
            constexpr static std::size_t value = first + Add<others...>::value;
        };


        /**
         * \brief Add std::size_t value without arithmetic overflow.
         *
         * Example:
         * \code
         * std::uint8_t buffer[dbor::SizeOf::Add<1, 2, 3>::value];
         * \endcode
         */
        template <std::size_t first>
        struct Add<first> {
            Add() = delete;
            constexpr static std::size_t value = first;
        };

    };

}

// Inline implementations ---

constexpr inline std::size_t dbor::SizeOf::integer(unsigned long long int value) noexcept {
    return
        value < 24ull
            ? 1u
            : value < 24ull + 0x100ull
                ? 2u
                : value < 24ull + 0x10100ull
                    ? 3u
                    : value < 24ull + 0x1010100ull
                        ? 4u
                        : value < 24ull + 0x101010100ull
                            ? 5u
                            : value < 24ull + 0x10101010100ull
                                ? 6u
                                : value < 24ull + 0x1010101010100ull
                                    ? 7u
                                    : value < 24ull + 0x101010101010100ull
                                        ? 8u
                                        : 9u;
}


constexpr inline std::size_t dbor::SizeOf::integer(unsigned long int value) noexcept {
    return integer(static_cast<unsigned long long int>(value));
}


constexpr inline std::size_t dbor::SizeOf::integer(unsigned int value) noexcept {
    return integer(static_cast<unsigned long long int>(value));
}


constexpr inline std::size_t dbor::SizeOf::integer(signed long long int value) noexcept {
    return integer(
        value < 0
            ? -(1ull + static_cast<unsigned long int>(value))
            : static_cast<unsigned long int>(value));
}


constexpr inline std::size_t dbor::SizeOf::integer(signed long int value) noexcept {
    return integer(static_cast<signed long long int>(value));
}


constexpr inline std::size_t dbor::SizeOf::integer(signed int value) noexcept {
    return integer(static_cast<signed long long int>(value));
}


constexpr inline std::size_t dbor::SizeOf::byteString(std::size_t stringSize) noexcept {
    return addSaturating(integer(stringSize), stringSize);
}


constexpr inline std::size_t dbor::SizeOf::byteString(
    std::initializer_list<std::uint8_t> bytes) noexcept
{
    return byteString(bytes.size());
}


constexpr inline std::size_t dbor::SizeOf::utf8String(std::size_t stringSize) noexcept {
    return addSaturating(integer(stringSize), stringSize);
}


constexpr inline std::size_t dbor::SizeOf::utf8String(const char *p) noexcept {
    return utf8String(stringSizeOf(p));
}


constexpr inline std::size_t dbor::SizeOf::addSaturating(std::size_t n, std::size_t m) noexcept {
    return m < std::numeric_limits<std::size_t>::max() - n
        ? m + n
        : std::numeric_limits<std::size_t>::max();
}


constexpr inline std::size_t dbor::SizeOf::stringSizeOf(const char *p) noexcept {
    static_assert(sizeof(char) == sizeof(std::uint8_t), "");
    return p == nullptr || p[0] == '\0' ? 0u : 1u + stringSizeOf(p + 1);
}


#endif  // DBOR_SIZEOF_HPP_