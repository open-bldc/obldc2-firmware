/*
 * Open-BLDC - Open BrushLess DC Motor Controller
 * Copyright (C) 2011 by Piotr Esden-Tempski <piotr@esden.net>
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
 * @file   mcu.c
 * @author Piotr Esden-Tempski <piotr@esden.net>
 * @date   Mar 18 2011
 *
 * @brief  mcu "driver" implementation.
 *
 * Implements functions for initializing global mcu specific features like the rcc.
 */

#include <libopencm3/stm32/f1/rcc.h>

#include "driver/mcu.h"

/**
 * Initialize STM32 system specific subsystems.
 */
void mcu_init(void) {
    /* Initialize the microcontroller system. Initialize clocks. */
    rcc_clock_setup_in_hsi_out_64mhz();
}

