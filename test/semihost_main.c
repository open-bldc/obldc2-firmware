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
 * @file   semihost_main.c
 * @author Piotr Esden-Tempski <piotr@esden.net>
 *
 * @brief  Semihosting test implementation.
 *
 */

#include <stdint.h>
#include <stdio.h>

#include "driver/mcu.h"
#include "driver/led.h"

void initialise_monitor_handles(void);

/**
 * Crude delay implementation.
 *
 * Just burn some MCU cycles
 *
 * @param delay "time" to wait
 */
static void my_delay(uint32_t delay) {
    while (delay != 0) {
        delay--;
        __asm("nop");
    }
}

/**
 * Semihosting test main function
 */
int main(void) {

    mcu_init();
    led_init();

#if SEMIHOSTING
    initialise_monitor_handles();
#endif

    while (1) {
        printf("Hello from semihosting.\n");
        my_delay(10000000);
    }
}
