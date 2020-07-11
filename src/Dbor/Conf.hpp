// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_CONF_HPP_
#define DBOR_CONF_HPP_

#include <cstdint>

#if !defined(DBOR_HAS_FAST_64BIT_ARITH)
    #if UINT_FAST32_MAX >= UINT64_MAX
        #define DBOR_HAS_FAST_64BIT_ARITH 1
    #else
        #define DBOR_HAS_FAST_64BIT_ARITH 0
    #endif
#endif

#endif  // DBOR_CONF_HPP_
