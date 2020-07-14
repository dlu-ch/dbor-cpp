// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "Dbor/Conf.hpp"
#include <limits>

// C++ "byte" as counted by sizeof() is exactly 8 bit:
static_assert(sizeof(std::uint8_t) == 1, "");
static_assert(sizeof(std::uint16_t) == 2, "");
static_assert(sizeof(std::uint32_t) == 4, "");
static_assert(sizeof(std::uint64_t) == 8, "");

static_assert(sizeof(std::size_t) == 2 || sizeof(std::size_t) == 4 || sizeof(std::size_t) == 8, "");

// each UTF-8 encoded std::uint8_t * with code points < 0x80 is a char *:
static_assert(sizeof(char) == sizeof(std::uint8_t), "");

static_assert(sizeof(float) == 4, "");
static_assert(std::numeric_limits<float>::is_iec559, "");
static_assert(std::numeric_limits<float>::has_denorm == std::denorm_present, "");

static_assert(sizeof(double) == 8, "");
static_assert(std::numeric_limits<double>::is_iec559, "");
static_assert(std::numeric_limits<double>::has_denorm == std::denorm_present, "");
