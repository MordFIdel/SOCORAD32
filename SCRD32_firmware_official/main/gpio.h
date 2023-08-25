#ifndef GPIO_H
#define GPIO_H

typedef struct {
    int timer_group;
    int timer_idx;
    int alarm_interval;
    bool auto_reload;
} example_timer_info_t;

#define ESP_INTR_FLAG_DEFAULT 0

void gpioTask(void *arg);

#endif