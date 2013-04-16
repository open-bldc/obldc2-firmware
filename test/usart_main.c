/*
 * Open-BLDC - Open BrushLess DC Motor Controller
 * Copyright (C) 2009-2013 by Piotr Esden-Tempski <piotr@esden.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   usart_main.c
 * @author Piotr Esden-Tempski <piotr@esden.net>
 *
 * @brief  USART driver test implementation.
 *
 */

#include <libopencm3/stm32/f1/gpio.h>

#include "driver/mcu.h"
#include "driver/led.h"
#include "driver/usart.h"

int8_t buffer[5];
int buffer_fill = 0;
bool buffer_full = false;

/**
 * USART byte handler function
 */
int usart_handle_byte(int8_t byte) {
    if (!buffer_full) {
        buffer[buffer_fill++] = byte;
        if (buffer_fill == 5) {
            buffer_full = true;
            usart_enable_send();
        }
    } else {
        return -1;
    }
    return 0;
}

/**
 * USART byte send handler function
 */
int32_t usart_get_byte(void) {
    int32_t ret = -1;

    if (buffer_full) {
        ret = buffer[--buffer_fill];
        if (buffer_fill == 0) {
            buffer_full = false;
        }
    }

    return ret;
}

/**
 * USART driver test main function
 */
int main(void) {

    mcu_init();
    led_init();
    usart_init(usart_handle_byte, usart_get_byte);

    while (1) __asm("nop");
}
