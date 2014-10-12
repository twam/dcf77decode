#ifndef ARITHMETIC_TOOLS_H
#define ARITHMETIC_TOOLS_H

//  www.blinkenlight.net
//
//  Copyright 2013 Udo Klein
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program. If not, see http://www.gnu.org/licenses/

#include <stdint.h>

namespace Arithmetic_Tools {
    template <uint8_t N> inline void bounded_increment(uint8_t &value) __attribute__((always_inline));
    template <uint8_t N>
    void bounded_increment(uint8_t &value) {
        if (value >= 255 - N) { value = 255; } else { value += N; }
    }

    template <uint8_t N> inline void bounded_decrement(uint8_t &value) __attribute__((always_inline));
    template <uint8_t N>
    void bounded_decrement(uint8_t &value) {
        if (value <= N) { value = 0; } else { value -= N; }
    }
}

#endif
