#include "dcf77_second_decoder.h"
#include "hamming.h"
#include "arithmetic_tools.h"
#include "dcf77.h"

namespace DCF77_Second_Decoder {
    using namespace DCF77;

    const uint8_t seconds_per_minute = 60;
    // this is a trick threshold
    //    lower it to get a faster second lock
    //    but then risk to garble the successive stages during startup
    //    --> to low and total startup time will increase
    const uint8_t lock_threshold = 12;

    typedef struct {
        uint8_t data[seconds_per_minute];
        uint8_t tick;

        uint8_t noise_max;
        uint8_t max;
        uint8_t max_index;
    } sync_bins;

    sync_bins bins;

    void sync_mark_binning(const uint8_t tick_data) {
        // We use a binning approach to find out the proper phase.
        // The goal is to localize the sync_mark. Due to noise
        // there may be wrong marks of course. The idea is to not
        // only look at the statistics of the marks but to exploit
        // additional data properties:

        // Bit position  0 after a proper sync is a 0.
        // Bit position 20 after a proper sync is a 1.

        // The binning will work as follows:

        //   1) A sync mark will score +6 points for the current bin
        //      it will also score -2 points for the previous bin
        //                         -2 points for the following bin
        //                     and -2 points 20 bins later
        //  In total this will ensure that a completely lost signal
        //  will not alter the buffer state (on average)

        //   2) A 0 will score +1 point for the previous bin
        //      it also scores -2 point 20 bins back
        //                 and -2 points for the current bin

        //   3) A 1 will score +1 point 20 bins back
        //      it will also score -2 point for the previous bin
        //                     and -2 points for the current bin

        //   4) An undefined value will score -2 point for the current bin
        //                                    -2 point for the previous bin
        //                                    -2 point 20 bins back

        //   5) Scores have an upper limit of 255 and a lower limit of 0.

        // Summary: sync mark earns 6 points, a 0 in position 0 and a 1 in position 20 earn 1 bonus point
        //          anything that allows to infer that any of the "connected" positions is not a sync will remove 2 points

        // It follows that the score of a sync mark (during good reception)
        // may move up/down the whole scale in slightly below 64 minutes.
        // If the receiver should glitch for whatever reason this implies
        // that the clock will take about 33 minutes to recover the proper
        // phase (during phases of good reception). During bad reception things
        // are more tricky.
        using namespace Arithmetic_Tools;

        const uint8_t previous_tick = bins.tick>0? bins.tick-1: seconds_per_minute-1;
        const uint8_t previous_21_tick = bins.tick>20? bins.tick-21: bins.tick + seconds_per_minute-21;

        switch (tick_data) {
            case sync_mark:
                bounded_increment<6>(bins.data[bins.tick]);

                bounded_decrement<2>(bins.data[previous_tick]);
                bounded_decrement<2>(bins.data[previous_21_tick]);

                { const uint8_t next_tick = bins.tick< seconds_per_minute-1? bins.tick+1: 0;
                bounded_decrement<2>(bins.data[next_tick]); }
                break;

            case short_tick:
                bounded_increment<1>(bins.data[previous_tick]);

                bounded_decrement<2>(bins.data[bins.tick]);
                bounded_decrement<2>(bins.data[previous_21_tick]);
                break;

            case long_tick:
                bounded_increment<1>(bins.data[previous_21_tick]);

                bounded_decrement<2>(bins.data[bins.tick]);
                bounded_decrement<2>(bins.data[previous_tick]);
                break;

            case undefined:
            default:
                bounded_decrement<2>(bins.data[bins.tick]);
                bounded_decrement<2>(bins.data[previous_tick]);
                bounded_decrement<2>(bins.data[previous_21_tick]);
        }
        bins.tick = bins.tick<seconds_per_minute-1? bins.tick+1: 0;

        // determine sync lock
        if (bins.max - bins.noise_max <=lock_threshold ||
            get_second() == 3) {
            // after a lock is acquired this happens only once per minute and it is
            // reasonable cheap to process,
            //
            // that is: after we have a "lock" this will be processed whenever
            // the sync mark was detected

            Hamming::compute_max_index(bins);
            }
    }

    void get_quality(Hamming::lock_quality_t &lock_quality) {
        Hamming::get_quality(bins, lock_quality);
    }

    uint8_t get_second() {
        if (bins.max - bins.noise_max >= lock_threshold) {
            // at least one sync mark and a 0 and a 1 seen
            // the threshold is tricky:
            //   higher --> takes longer to acquire an initial lock, but higher probability of an accurate lock
            //
            //   lower  --> higher probability that the lock will oscillate at the beginning
            //              and thus spoil the downstream stages

            // we have to subtract 2 seconds
            //   1 because the seconds already advanced by 1 tick
            //   1 because the sync mark is not second 0 but second 59

            uint8_t second = 2*seconds_per_minute + bins.tick - 2 - bins.max_index;
            while (second >= seconds_per_minute) { second-= seconds_per_minute; }

            return second;
        } else {
            return 0xff;
        }
    }

    void process_single_tick_data(const uint8_t tick_data) {
        sync_mark_binning(tick_data);
    }

    void setup() {
        Hamming::setup(bins);
    }

    void debug() {
        static uint8_t prev_tick;

        if (prev_tick == bins.tick) {
            return;
        } else {
            prev_tick = bins.tick;

            printf("DCF77_Second_Decoder: Second: ");
            printf("%u", get_second());
            printf(" SyncMarkIndex: ");
            Hamming::debug(bins);
        }
    }
}