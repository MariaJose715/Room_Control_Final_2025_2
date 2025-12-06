#ifndef RFID_DRIVER_H
#define RFID_DRIVER_H

#include "main.h"
#include <stdint.h>

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
} rfid_handle_t;

void rfid_init(rfid_handle_t *rfid);
uint8_t rfid_is_card_present(rfid_handle_t *rfid);
uint8_t rfid_read_uid(rfid_handle_t *rfid, uint8_t *uid_buffer);

#endif
