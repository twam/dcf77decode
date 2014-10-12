#ifndef HAMMING_H
#define HAMMING_H

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
#include <stdio.h>

namespace Hamming {
    typedef struct {
        uint8_t lock_max;
        uint8_t noise_max;
    } lock_quality_t;
}

namespace Hamming {
    template <typename bins_t>
    void advance_tick(bins_t &bins) {
        const uint8_t number_of_bins = sizeof(bins.data) / sizeof(bins.data[0]);
        if (bins.tick < number_of_bins - 1) {
            ++bins.tick;
        } else {
            bins.tick = 0;
        }
    }

    template <typename bins_t>
    void compute_max_index(bins_t &bins) {
        const uint8_t number_of_bins = sizeof(bins.data) / sizeof(bins.data[0]);

        bins.noise_max = 0;
        bins.max = 0;
        bins.max_index = 255;
        for (uint8_t index = 0; index < number_of_bins; ++index) {
            const uint8_t bin_data = bins.data[index];

            if (bin_data >= bins.max) {
                bins.noise_max = bins.max;
                bins.max = bin_data;
                bins.max_index = index;
            } else if (bin_data > bins.noise_max) {
                bins.noise_max = bin_data;
            }
        }
    }

    template <typename bins_t>
    void setup(bins_t &bins) {
        const uint8_t number_of_bins = sizeof(bins.data) / sizeof(bins.data[0]);

        for (uint8_t index = 0; index < number_of_bins; ++index) {
            bins.data[index] = 0;
        }
        bins.tick = 0;

        bins.max = 0;
        bins.max_index = 255;
        bins.noise_max = 0;
    }

    template <typename bins_t>
    uint8_t get_time_value(const bins_t &bins) {
        // there is a trade off involved here:
        //    low threshold --> lock will be detected earlier
        //    low threshold --> if lock is not clean output will be garbled
        //    a proper lock will fix the issue
        //    the question is: which start up behaviour do we prefer?
        const uint8_t threshold = 2;

        const uint8_t number_of_bins = sizeof(bins.data) / sizeof(bins.data[0]);

        if (bins.max-bins.noise_max >= threshold) {
            return (bins.max_index + bins.tick + 1) % number_of_bins;
        } else {
            const uint8_t undefined = 0xff;
            return undefined;
        }
    }

    template <typename bins_t>
    void get_quality(const bins_t bins, Hamming::lock_quality_t &lock_quality) {
        lock_quality.lock_max = bins.max;
        lock_quality.noise_max = bins.noise_max;
    }

    template <typename bins_t>
    void debug (const bins_t &bins) {
        const uint8_t number_of_bins = sizeof(bins.data) / sizeof(bins.data[0]);
        const bool uses_integrals = sizeof(bins.max) == 4;

        printf("0x%02X", get_time_value(bins));
        printf(" Tick: ");
        printf("%u", bins.tick);
        printf(" Quality: ");
        printf("%lu", (uint32_t)bins.max);
        printf("-");
        printf("%lu", (uint32_t)bins.noise_max);
        printf(" MaxIndex: ");
        printf("%u", bins.max_index);
        printf(" > ");

        for (uint8_t index = 0; index < number_of_bins; ++index) {
            if (index == bins.max_index ||
                (!uses_integrals && index == (bins.max_index+1) % number_of_bins) ||
                (uses_integrals && (index == (bins.max_index+10) % number_of_bins ||
                (index == (bins.max_index+20) % number_of_bins)))) {
                printf("| ");
                }
                printf("0x%04X ", bins.data[index]);
        }
        printf("\n");
    }
}

#endif