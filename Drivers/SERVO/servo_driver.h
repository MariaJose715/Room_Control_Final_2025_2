#ifndef SERVO_DRIVER_H
#define SERVO_DRIVER_H

#include "main.h"

typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    uint16_t min_pulse; 
    uint16_t max_pulse;
} servo_handle_t;

void servo_init(servo_handle_t *servo);
void servo_write_angle(servo_handle_t *servo, uint8_t angle);

#endif
