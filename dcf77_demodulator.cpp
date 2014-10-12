#include "dcf77_demodulator.h"

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

#include <stdio.h>
#include "hamming.h"
#include "dcf77.h"
#include "dcf77_clock_controller.h"

namespace DCF77_Demodulator {
    using namespace DCF77;

    const uint8_t bin_count = 100;

    typedef struct {
        uint16_t data[bin_count];
        uint8_t tick;

        uint32_t noise_max;
        uint32_t max;
        uint8_t max_index;
    } phase_bins;

    phase_bins bins;

    const uint16_t samples_per_second = 1000;

    const uint16_t samples_per_bin = samples_per_second / bin_count;
    const uint16_t bins_per_10ms  = bin_count / 100;
    const uint16_t bins_per_50ms  =  5 * bins_per_10ms;
    const uint16_t bins_per_60ms  =  6 * bins_per_10ms;
    const uint16_t bins_per_100ms = 10 * bins_per_10ms;
    const uint16_t bins_per_200ms = 20 * bins_per_10ms;
    const uint16_t bins_per_500ms = 50 * bins_per_10ms;

    void setup() {
        Hamming::setup(bins);

        for (uint8_t index = 0; index < bin_count; ++index) {
            bins.data[index] = 0;
        }
        bins.tick = 0;

        bins.max = 0;
        bins.max_index = 255;
        bins.noise_max = 0;
    }

    void decode_220ms(const uint8_t input, const uint8_t bins_to_go) {
        // will be called for each bin during the "interesting" 220ms

        static uint8_t count = 0;
        static uint8_t decoded_data = 0;

        count += input;
        if (bins_to_go >= bins_per_100ms + bins_per_10ms) {
            if (bins_to_go == bins_per_100ms + bins_per_10ms) {
                decoded_data = count > bins_per_50ms? 2: 0;
                count = 0;
            }
        } else {
            if (bins_to_go == 0) {
                decoded_data += count > bins_per_50ms? 1: 0;
                count = 0;
                // pass control further
                // decoded_data: 3 --> 1
                //               2 --> 0,
                //               1 --> undefined,
                //               0 --> sync_mark
                DCF77_Clock_Controller::process_single_tick_data(decoded_data);
            }
        }
    }

    uint16_t wrap(const uint16_t value) {
        // faster modulo function which avoids division
        uint16_t result = value;
        while (result >= bin_count) {
            result-= bin_count;
        }
        return result;
    }

    void phase_detection() {
        // We will compute the integrals over 200ms.
        // The integrals is used to find the window of maximum signal strength.
        uint32_t integral = 0;

        for (uint16_t bin = 0; bin < bins_per_100ms; ++bin)  {
            integral += ((uint32_t)bins.data[bin])<<1;
        }

        for (uint16_t bin = bins_per_100ms; bin < bins_per_200ms; ++bin)  {
            integral += (uint32_t)bins.data[bin];
        }

        bins.max = 0;
        bins.max_index = 0;
        for (uint16_t bin = 0; bin < bin_count; ++bin) {
            if (integral > bins.max) {
                bins.max = integral;
                bins.max_index = bin;
            }

            integral -= (uint32_t)bins.data[bin]<<1;
            integral += (uint32_t)(bins.data[wrap(bin + bins_per_100ms)] + bins.data[wrap(bin + bins_per_200ms)]);
        }

        // max_index indicates the position of the 200ms second signal window.
        // Now how can we estimate the noise level? This is very tricky because
        // averaging has already happened to some extend.

        // The issue is that most of the undesired noise happens around the signal,
        // especially after high->low transitions. So as an approximation of the
        // noise I test with a phase shift of 200ms.
        bins.noise_max = 0;
        const uint16_t noise_index = wrap(bins.max_index + bins_per_200ms);

        for (uint16_t bin = 0; bin < bins_per_100ms; ++bin)  {
            bins.noise_max += ((uint32_t)bins.data[wrap(noise_index + bin)])<<1;
        }

        for (uint16_t bin = bins_per_100ms; bin < bins_per_200ms; ++bin)  {
            bins.noise_max += (uint32_t)bins.data[wrap(noise_index + bin)];
        }
    }

    uint8_t phase_binning(const uint8_t input) {
        // how many seconds may be cummulated
        // this controls how slow the filter may be to follow a phase drift
        // N times the clock precision shall be smaller 1
        // clock 30 ppm => N < 300
        const uint16_t N = 300;

        Hamming::advance_tick(bins);

        if (input) {
            if (bins.data[bins.tick] < N) {
                ++bins.data[bins.tick];
            }
        } else {
            if (bins.data[bins.tick] > 0) {
                --bins.data[bins.tick];
            }
        }
        return bins.tick;
    }

    void detector_stage_2(const uint8_t input) {
        const uint8_t current_bin = bins.tick;

        const uint8_t threshold = 30;

        if (bins.max-bins.noise_max < threshold ||
            wrap(bin_count + current_bin - bins.max_index) == 53) {
            // Phase detection far enough out of phase from anything that
            // might consume runtime otherwise.
            phase_detection();
        }

        static uint8_t bins_to_process = 0;

        if (bins_to_process == 0) {
            if (wrap((bin_count + current_bin - bins.max_index)) <= bins_per_100ms ||   // current_bin at most 100ms after phase_bin
                wrap((bin_count + bins.max_index - current_bin)) <= bins_per_10ms ) {   // current bin at most 10ms before phase_bin
                // if phase bin varies to much during one period we will always be screwed in may ways...

                // last 10ms of current second
                DCF77_Clock_Controller::flush();

                // start processing of bins
                bins_to_process = bins_per_200ms + 2*bins_per_10ms;
            }
        }

        if (bins_to_process > 0) {
            --bins_to_process;

            // this will be called for each bin in the "interesting" 220ms
            // this is also a good place for a "monitoring hook"
            decode_220ms(input, bins_to_process);
        }
    }

    void detector(const uint8_t sampled_data) {
        static uint8_t current_sample = 0;
        static uint8_t average = 0;

        // detector stage 0: average 10 samples (per bin)
        average += sampled_data;

        if (++current_sample >= samples_per_bin) {
            // once all samples for the current bin are captured the bin gets updated
            // that is each 10ms control is passed to stage 1
            const uint8_t input = (average> samples_per_bin/2);

            phase_binning(input);

            detector_stage_2(input);

            average = 0;
            current_sample = 0;
        }
    }

    void get_quality(uint32_t &lock_max, uint32_t &noise_max) {
        lock_max = bins.max;
        noise_max = bins.noise_max;
    }

    void debug() {
        printf("DCF77_Demodulator: Phase: ");
        Hamming::debug(bins);
    }
}