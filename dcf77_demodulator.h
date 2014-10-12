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
    const uint8_t bin_count = 100;

    typedef struct {
        uint16_t data[bin_count];
        uint8_t tick;

        uint32_t noise_max;
        uint32_t max;
        uint8_t max_index;
    } phase_bins;

    extern phase_bins bins;

    const uint16_t samples_per_second = 1000;

    const uint16_t samples_per_bin = samples_per_second / bin_count;
    const uint16_t bins_per_10ms  = bin_count / 100;
    const uint16_t bins_per_50ms  =  5 * bins_per_10ms;
    const uint16_t bins_per_60ms  =  6 * bins_per_10ms;
    const uint16_t bins_per_100ms = 10 * bins_per_10ms;
    const uint16_t bins_per_200ms = 20 * bins_per_10ms;
    const uint16_t bins_per_500ms = 50 * bins_per_10ms;

    void setup(void);
    uint16_t wrap(const uint16_t value);
    void phase_detection(void);
    void advance_tick(void);
    uint8_t phase_binning(const uint8_t input);
    void detector_stage_2(const uint8_t input);
    void detector(const uint8_t sampled_data);
    void get_quality(uint32_t &lock_max, uint32_t &noise_max);
    uint8_t get_time_value();
}

#endif