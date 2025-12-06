#include "servo_driver.h"
#include "stm32l4xx_hal.h"

void servo_init(servo_handle_t *servo) {
    HAL_TIM_PWM_Start(servo->htim, servo->channel);
}

void servo_write_angle(servo_handle_t *servo, uint8_t angle) {
    if (angle > 180) angle = 180;

    uint32_t pulse = servo->min_pulse +
                     (angle * (servo->max_pulse - servo->min_pulse)) / 180;

    __HAL_TIM_SET_COMPARE(servo->htim, servo->channel, pulse);
}
