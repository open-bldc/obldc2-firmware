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
 * @file   sys_tick_main.c
 * @author Piotr Esden-Tempski <piotr@esden.net>
 *
 * @brief  Sys Tick soft timer test implementation.
 *
 */

#include <libopencm3/stm32/f1/gpio.h>

#include "driver/mcu.h"
#include "driver/led.h"
#include "driver/timer.h"

static void timer_callback(int id, uint16_t time);

/**
 * One shot callback from the soft Sys Tick timer
 */
void timer_callback(__attribute__((unused)) int id, __attribute__((unused)) uint16_t time) {
    TOGGLE(LED_RED);
}

/**
 * Sys Tick soft timer test main function
 */
int main(void) {

    mcu_init();
    led_init();
    timer_init();

    /* 1kHz timer callback. */
    (void)timer_register(4000, timer_callback);

    while (true) {
        __asm("nop");
    }
}
