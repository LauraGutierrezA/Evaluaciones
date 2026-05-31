#ifndef __TIMERASDELAY_H__
#define __TIMERASDELAY_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/timer.h"

class TimerAsDelay {
    public:
        TimerAsDelay(timer_group_t group_num, timer_idx_t timer_num);
        ~TimerAsDelay();
        void init(void);
        bool delay_s(uint32_t s);
        bool delay_ms(uint32_t ms);
        bool delay_us(uint32_t us);

    private:
        timer_group_t _groupNum;
        timer_idx_t _timerNum;
        uint64_t _last;
};


#endif // __TIMERASDELAY_H__