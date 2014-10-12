#include "dcf77_clock_controller.h"
#include "dcf77_second_decoder.h"
#include "dcf77_demodulator.h"

namespace DCF77_Clock_Controller {
    flush_handler output_handler = 0;

    DCF77::time_data decoded_time;
    volatile bool second_toggle;

    void get_current_time(DCF77::time_data &now) {
        for (bool stopper = second_toggle; stopper == second_toggle; ) {
            // wait for second_toggle to toggle
            // that is wait for decoded time to be ready
        }
        now = decoded_time;
    }

    void flush() {
        // this is called at the end of each second / before the next second begins
        // it is most interesting to propagate this further at the end of a sync marks
        // it is also interesting to propagate this to reference clocks

        decoded_time.second = DCF77_Second_Decoder::get_second();
        second_toggle = !second_toggle;

        // decoded_time holds the value of the current second but
        // we are immediately before the start of the next
        // second --> add 1 second to decoded_time

        if (decoded_time.second < 60) {
            decoded_time.second = (decoded_time.second + 1) % 60;
        }

        if (output_handler) {
            output_handler(decoded_time);
        }
    }

    void set_flush_handler(const flush_handler new_output_handler) {
        output_handler = new_output_handler;
    }

    void get_quality(clock_quality_t &clock_quality) {
        DCF77_Demodulator::get_quality(clock_quality.phase.lock_max, clock_quality.phase.noise_max);
        DCF77_Second_Decoder::get_quality(clock_quality.second);
    }

    void setup() {
        DCF77_Second_Decoder::setup();
    }

    void process_single_tick_data(const uint8_t tick_data) {
        DCF77_Second_Decoder::process_single_tick_data(tick_data);
    }
}
