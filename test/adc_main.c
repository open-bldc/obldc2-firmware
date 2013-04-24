/*
 * Open-BLDC - Open BrushLess DC Motor Controller
 * Copyright (C) 2013 by Piotr Esden-Tempski <piotr@esden.net>
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
 * @file   adc_main.c
 * @author Piotr Esden-Tempski <piotr@esden.net>
 *
 * @brief  ADC test implementation
 *
 */

#include "driver/mcu.h"
#include "driver/led.h"
#include "driver/adc.h"
#include "driver/pwm.h"

/**
 * ADC test main function
 *
 * @return Nothing really...
 */
int main(void) {
    mcu_init();
    led_init();
    adc_init();
    pwm_init();

    while (true) {
        __asm("nop");
    }
}
