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
 * @file   pwm_comm_main.c
 * @author Piotr Esden-Tempski <piotr@esden.net>
 *
 * @brief  PWM commutation test implementation
 *
 */

#include <libopencm3/stm32/f1/gpio.h>

#include "driver/mcu.h"
#include "driver/led.h"
#include "driver/pwm.h"

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
 * Blink test main function
 *
 * @return Nothing really...
 */
int main(void)
{
	mcu_init();
	led_init();
	pwm_init();

	/* Set PWM to 10% positive power. */
	pwm_set(INT16_MAX/10);

	/* Continously commutate. With some arbitrary frequency. */
	while (true) {
		pwm_comm();
		my_delay(100000);
		TOGGLE(LED_RED);
	}
}
