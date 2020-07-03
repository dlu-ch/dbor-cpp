// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#ifndef DBOR_TYPE_HPP_
#define DBOR_TYPE_HPP_

#include <cstdint>

namespace dbor {

    enum class ErrorCode {
        OK,
        IMPRECISE,
        OUT_OF_RANGE,
        NO_OBJECT,
        INCOMPATIBLE,
        ILLFORMED
    };

};


#endif  // DBOR_TYPE_HPP_
