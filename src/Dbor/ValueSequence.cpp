// SPDX-License-Identifier: LGPL-3.0-or-later
// dbor-c++ - C++ implementation of DBOR encoder and decoder
// Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>


#include "Dbor/ValueSequence.hpp"
#include "Dbor/Encoding.hpp"

using namespace dbor;


ValueSequence::Iterator::Iterator() noexcept
    : remainingSize_(0)
{
}


ValueSequence::Iterator::Iterator(const void *buffer, std::size_t capacity) noexcept
    : front_(static_cast<const uint8_t *>(buffer), capacity)
{
    remainingSize_ = front_.buffer() ? capacity - front_.size() : 0;
}


ValueSequence::Iterator &ValueSequence::Iterator::operator++() noexcept {
    if (remainingSize_) {
        front_ = Value(front_.buffer() + front_.size(), remainingSize_);
        remainingSize_ -= front_.size();
    } else {
        // next is end()
        front_ = Value();
    }
    return *this;
}
