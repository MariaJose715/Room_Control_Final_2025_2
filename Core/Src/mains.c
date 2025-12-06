/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>
#include <stdio.h>

#include "ring_buffer.h"
#include "keypad_driver.h"
#include "rfid_driver.h"
#include "servo_driver.h"

/* Periféricos externos (generados por STM32CubeMX) */
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart2;

/* --- CONFIGURACIÓN KEYPAD --- */
#define KEYPAD_BUFFER_LEN 16
uint8_t keypad_data[KEYPAD_BUFFER_LEN];
ring_buffer_t keypad_rb;

keypad_handle_t keypad = {
    .row_ports = {KEYPAD_R1_GPIO_Port, KEYPAD_R2_GPIO_Port, KEYPAD_R3_GPIO_Port, KEYPAD_R4_GPIO_Port},
    .row_pins  = {KEYPAD_R1_Pin, KEYPAD_R2_Pin, KEYPAD_R3_Pin, KEYPAD_R4_Pin},
    .col_ports = {KEYPAD_C1_GPIO_Port, KEYPAD_C2_GPIO_Port, KEYPAD_C3_GPIO_Port, KEYPAD_C4_GPIO_Port},
    .col_pins  = {KEYPAD_C1_Pin, KEYPAD_C2_Pin, KEYPAD_C3_Pin, KEYPAD_C4_Pin}
};

/* --- CONFIGURACIÓN RFID --- */
rfid_handle_t rfid = {
    .hspi = &hspi1,
    .cs_port = RFID_CS_GPIO_Port,
    .cs_pin  = RFID_CS_Pin,
    .rst_port = RFID_RST_GPIO_Port,
    .rst_pin  = RFID_RST_Pin
};

/* Lista de UIDs permitidos (ajusta según tus tarjetas) */
uint8_t allowed_uids[][4] = {
    {0xDE,0xAD,0xBE,0xEF},
    {0xA1,0xB2,0xC3,0xD4}
};

/* --- CONFIGURACIÓN SERVO --- */
servo_handle_t servo = {
    .htim = &htim3,
    .channel = TIM_CHANNEL_1,
    .min_pulse = 1000,
    .max_pulse = 2000
};

/* Callback para EXTI (teclado) — cada vez que se presiona una tecla */
void HAL_GPIO_EXTI_Callback(uint16_t pin) {
    char key = keypad_scan(&keypad, pin);
    if (key != '\0') {
        ring_buffer_write(&keypad_rb, (uint8_t)key);
    }
}

/* Función auxiliar para imprimir por UART (consola) */
static void print(const char *s) {
    HAL_UART_Transmit(&huart2, (uint8_t*)s, strlen(s), 100);
}

/* Verifica si UID leido está en la lista de permitidos */
static int uid_ok(uint8_t *uid) {
    for (int i = 0; i < (int)(sizeof(allowed_uids)/4); i++) {
        if (memcmp(uid, allowed_uids[i], 4) == 0) {
            return 1;
        }
    }
    return 0;
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_SPI1_Init();
    MX_TIM3_Init();
    MX_USART2_UART_Init();

    /* Inicializaciones de librerías y periféricos */
    ring_buffer_init(&keypad_rb, keypad_data, KEYPAD_BUFFER_LEN);
    keypad_init(&keypad);
    rfid_init(&rfid);
    servo_init(&servo);
    servo_write_angle(&servo, 0); // Servo en posición “cerrado” inicialmente

    print("Sistema Control Sala - RFID + Servo listo!\r\n");

    uint8_t uid[4];

    while (1) {
        /* --- LEE TECLADO --- */
        uint8_t k;
        if (ring_buffer_read(&keypad_rb, &k)) {
            char txt[32];
            sprintf(txt, "Tecla: %c\r\n", k);
            print(txt);
            // Aquí puedes añadir lógica adicional según tecla
        }

        /* --- LEE RFID --- */
        if (rfid_is_card_present(&rfid)) {
            if (rfid_read_uid(&rfid, uid)) {
                char txt[64];
                sprintf(txt, "UID: %02X %02X %02X %02X\r\n", uid[0], uid[1], uid[2], uid[3]);
                print(txt);

                if (uid_ok(uid)) {
                    print("ACCESO PERMITIDO - Abriendo puerta...\r\n");
                    servo_write_angle(&servo, 90); // Ángulo de apertura
                    HAL_Delay(3500);               // Tiempo de puerta abierta
                    servo_write_angle(&servo, 0);  // Cerrar puerta
                    print("Puerta cerrada.\r\n");
                } else {
                    print("UID NO PERMITIDO.\r\n");
                }

                HAL_Delay(500); // Evita detección repetida inmediata
            }
        }

        HAL_Delay(40);
    }
}
