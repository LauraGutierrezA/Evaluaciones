#include "TIMERASDELAY.h"

TimerAsDelay::TimerAsDelay(timer_group_t group_num, timer_idx_t timer_num) {
    _groupNum = group_num;
    _timerNum = timer_num;
    uint64_t _last = 0;
}

TimerAsDelay::~TimerAsDelay() {
    timer_deinit(_groupNum, _timerNum);
}

void TimerAsDelay::init(void) {
    timer_config_t config = {};
    config.divider = 80; // Prescaler para contar en microsegundos (80MHz / 80 = 1MHz)
    config.counter_dir = TIMER_COUNT_UP;
    config.counter_en = TIMER_PAUSE;
    config.alarm_en = TIMER_ALARM_DIS;
    config.auto_reload = TIMER_AUTORELOAD_EN;

    timer_init(_groupNum, _timerNum, &config);
    timer_set_counter_value(_groupNum, _timerNum, 0);
    timer_start(_groupNum, _timerNum);
    
}

bool TimerAsDelay::delay_s(uint32_t s) {
    uint64_t now;
    timer_get_counter_value(_groupNum, _timerNum, &now); 

    if (now - _last >= s * 1000000) {
        _last = now;
        return true;
    }
    return false;
}

bool TimerAsDelay::delay_ms(uint32_t ms) {
    uint64_t now;
    timer_get_counter_value(_groupNum, _timerNum, &now); 

    if (now - _last >= ms * 1000) {
        _last = now;
        return true;
    }
    return false;
}

bool TimerAsDelay::delay_us(uint32_t us) {
    uint64_t now;
    timer_get_counter_value(_groupNum, _timerNum, &now); 

    if (now - _last >= us) {
        _last = now;
        return true;
    }
    return false;
}