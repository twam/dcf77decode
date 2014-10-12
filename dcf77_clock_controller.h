#ifndef DCF77_CLOCK_CONTROLLER_H
#define DCF77_CLOCK_CONTROLLER_H

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
#include "hamming.h"
#include "dcf77.h"

namespace DCF77_Clock_Controller {
    void setup();
    void process_single_tick_data(const uint8_t tick_data);

    void flush();
    typedef void (*flush_handler)(const DCF77::time_data &decoded_time);

    void default_handler(const DCF77::time_data &decoded_time);
    void set_flush_handler(const flush_handler output_handler);

    typedef Hamming::lock_quality_t lock_quality_t;

    typedef struct {
        struct {
            uint32_t lock_max;
            uint32_t noise_max;
        } phase;

        lock_quality_t second;
    } clock_quality_t;

    void get_quality(clock_quality_t &clock_quality);

    // blocking, will unblock at the start of the second
    void get_current_time(DCF77::time_data &now);
}

#endif