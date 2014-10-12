#ifndef DCF77_H
#define DCF77_H

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

namespace DCF77 {
    const uint8_t long_tick  = 3;
    const uint8_t short_tick = 2;
    const uint8_t undefined  = 1;
    const uint8_t sync_mark  = 0;

    typedef struct {
        uint8_t second;      // 0..60
    } time_data;
}

#endif