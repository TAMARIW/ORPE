#ifndef TIMINGCONTROL_HPP
#define TIMINGCONTROL_HPP

/**
 * This contains a helper class that helps control timing.
*/

#include "stdint.h"
#include "stddef.h"

#include 



/**
 * This class is used to control timing intervals.
 * 
*/
class TimingControl {
private:




public:

    TimingControl();

    /**
     * @brief Sets the interval for the timing.
     * @param time The current absolute time in microseconds.
    */
    void setInterval(int64_t interval_us);

    /**
     * 
    */
    void setRepeatLimit(size_t repeatLimit);

    /**
     * @brief Will calculate the current delta from the trigger interval. Negative means there is time left, positive means its past.
     * @param time The current absolute time in microseconds.
    */
    int64_t getDelta(int64_t time_us) const;

    /**
     * @brief Check if the timing has been triggered.
     * @param time The current absolute time in microseconds. 
     * @param reset If the timer should be reset. Set to true if you want to wait for the next interval.
     * @param rebaseTiming If true, then the following trigger interval will be the same as the s
     * @return true if the interval has been reached. 
    */
    bool isTriggered(int64_t time_us, bool reset = false, bool rebaseTiming = false);

    /**
     * @brief Will reset timer for the next interval.
    */
    void reset(int64_t time_us, bool rebaseTiming = false);


};











#endif 