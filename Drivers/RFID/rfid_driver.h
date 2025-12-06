#ifndef RFID_DRIVER_H
#define RFID_DRIVER_H

#include "main.h"
#include <stdbool.h>

typedef struct {
    SPI_HandleTypeDef *hspi;
    GPIO_TypeDef *cs_port;
    uint16_t cs_pin;
    GPIO_TypeDef *rst_port;
    uint16_t rst_pin;
} rfid_handle_t;

void rfid_init(rfid_handle_t *rfid);
bool rfid_is_card_present(rfid_handle_t *rfid);
bool rfid_read_uid(rfid_handle_t *rfid, uint8_t *uid_out);

#endif
