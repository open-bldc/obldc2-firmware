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
 * @file   blink_main.c
 * @author Piotr Esden-Tempski <piotr@esden.net>
 *
 * @brief  Blink test implementation
 *
 */

#include <libopencm3/stm32/f1/gpio.h>

#include "driver/mcu.h"
#include "driver/led.h"

/**
 * Crude delay implementation.
 *
 * Just burn some MCU cycles
 *
 * @param delay "time" to wait
 */
static void my_delay(uint32_t delay)
{
	while (delay != 0) {
		delay--;
		__asm("nop");
	}
}

/**
 * Turn on LED by id.
 *
 * @param id LED id
 */
static void led_on(int id)
{
	switch (id) {
	case 0:
		ON(LED_RED);
		break;
	case 1:
		ON(LED_GREEN);
		break;
	}
}

/**
 * Turn off LED by id
 *
 * @param id LED id
 */
static void led_off(int id)
{
	switch (id) {
	case 0:
		OFF(LED_RED);
		break;
	case 1:
		OFF(LED_GREEN);
		break;
	}
}

/**
 * Blink test main function
 *
 * @return Nothing really...
 */
int main(void)
{
	int i, j, led_id;

	mcu_init();
	led_init();

	led_id = 0;

	while (true) {
		for (j = 0; j < 20; j++) {
			for (i = 0; i < 125; i++) {
				led_on(led_id);
				my_delay((uint32_t)(50 * j));
				led_off(led_id);
				my_delay((uint32_t)(1200 - 50 * j));
			}
		}
		for (j = 0; j < 20; j++) {
			for (i = 0; i < 125; i++) {
				led_off(led_id);
				my_delay((uint32_t)(200 + 50 * j));
				led_on(led_id);
				my_delay((uint32_t)(1000 - 50 * j));
			}
		}
		led_off(led_id);
		led_id++;
		led_id %= 2;
	}
}
