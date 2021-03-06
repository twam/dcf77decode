#ifndef DCF77_DEMODULATOR_H
#define DCF77_DEMODULATOR_H

//  www.blinkenlight.net
//
//  Copyright 2012 Udo Klein
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

namespace DCF77_Demodulator {
    void setup();
    void detector(const uint8_t sampled_data);
    void get_quality(uint32_t &lock_max, uint32_t &noise_max);
    void get_noise_indicator(uint32_t &noise_indicator);

    void debug();
}

#endif