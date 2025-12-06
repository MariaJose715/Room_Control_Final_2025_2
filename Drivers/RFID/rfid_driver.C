#include "rfid_driver.h"
#include "stm32l4xx_hal.h"
#include <string.h>

#define PCD_IDLE        0x00
#define PCD_TRANSCEIVE  0x0C
#define PCD_SOFTRESET   0x0F
#define PICC_REQIDL     0x26
#define PICC_ANTICOLL   0x93

#define REG_COMMAND     0x01
#define REG_FIFO        0x09
#define REG_FIFOLVL     0x0A
#define REG_BITFRAMING  0x0D
#define REG_TXCONTROL   0x14
#define REG_COMMIRQ     0x04
#define REG_ERROR       0x06

static inline void cs_low(rfid_handle_t *rfid) {
    HAL_GPIO_WritePin(rfid->cs_port, rfid->cs_pin, GPIO_PIN_RESET);
}
static inline void cs_high(rfid_handle_t *rfid) {
    HAL_GPIO_WritePin(rfid->cs_port, rfid->cs_pin, GPIO_PIN_SET);
}

static void write_reg(rfid_handle_t *rfid, uint8_t reg, uint8_t val) {
    uint8_t tx[2] = { (reg << 1) & 0x7E, val };
    cs_low(rfid);
    HAL_SPI_Transmit(rfid->hspi, tx, 2, HAL_MAX_DELAY);
    cs_high(rfid);
}

static uint8_t read_reg(rfid_handle_t *rfid, uint8_t reg) {
    uint8_t tx = ((reg << 1) & 0x7E) | 0x80;
    uint8_t rx;
    cs_low(rfid);
    HAL_SPI_Transmit(rfid->hspi, &tx, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(rfid->hspi, &rx, 1, HAL_MAX_DELAY);
    cs_high(rfid);
    return rx;
}

static void antenna_on(rfid_handle_t *rfid) {
    uint8_t val = read_reg(rfid, REG_TXCONTROL);
    if (!(val & 0x03)) write_reg(rfid, REG_TXCONTROL, val | 0x03);
}

void rfid_init(rfid_handle_t *rfid) {
    HAL_GPIO_WritePin(rfid->rst_port, rfid->rst_pin, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(rfid->rst_port, rfid->rst_pin, GPIO_PIN_SET);
    HAL_Delay(5);

    write_reg(rfid, REG_COMMAND, PCD_SOFTRESET);
    HAL_Delay(50);

    write_reg(rfid, REG_BITFRAMING, 0x00);
    antenna_on(rfid);
}

bool rfid_is_card_present(rfid_handle_t *rfid) {
    uint8_t cmd = PICC_REQIDL;
    uint8_t back[8];
    uint8_t blen = 8;

    write_reg(rfid, REG_FIFOLVL, 0x80);
    write_reg(rfid, REG_FIFO, cmd);
    write_reg(rfid, REG_COMMAND, PCD_TRANSCEIVE);
    write_reg(rfid, REG_BITFRAMING, 0x80);

    HAL_Delay(10);

    uint8_t lvl = read_reg(rfid, REG_FIFOLVL);
    return lvl > 0;
}

bool rfid_read_uid(rfid_handle_t *rfid, uint8_t *uid_out) {
    uint8_t cmd[2] = { PICC_ANTICOLL, 0x20 };

    write_reg(rfid, REG_FIFOLVL, 0x80);
    write_reg(rfid, REG_FIFO, cmd[0]);
    write_reg(rfid, REG_FIFO, cmd[1]);

    write_reg(rfid, REG_COMMAND, PCD_TRANSCEIVE);
    write_reg(rfid, REG_BITFRAMING, 0x80);
    HAL_Delay(10);

    uint8_t lvl = read_reg(rfid, REG_FIFOLVL);
    if (lvl < 5) return false;

    for (int i = 0; i < 4; i++)
        uid_out[i] = read_reg(rfid, REG_FIFO);

    return true;
}
