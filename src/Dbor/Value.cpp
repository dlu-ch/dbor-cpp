// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

#include "Dbor/Value.hpp"
#include "Dbor/Encoding.hpp"

using namespace dbor;


Value::Value() noexcept
    : buffer_(nullptr)
    , size_(0)
{
}


Value::Value(const uint8_t *buffer, std::size_t capacity) noexcept {
    if (buffer && capacity) {
        size_ = Encoding::sizeOfValueIn(buffer, capacity);
        if (size_ == 0 || size_ > capacity)
            size_ = capacity;
        buffer_ = buffer;
    } else {
        buffer_ = nullptr;
        size_ = 0;
    }
}
