// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "Dbor/Conf.hpp"

// C++ "byte" as counted by sizeof() is exactly 8 bit:
static_assert(sizeof(std::uint8_t) == 1, "");
static_assert(sizeof(std::uint16_t) == 2, "");
static_assert(sizeof(std::uint32_t) == 4, "");

static_assert(sizeof(std::size_t) == 2 || sizeof(std::size_t) == 4 || sizeof(std::size_t) == 8, "");
