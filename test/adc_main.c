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

uint32_t uv, vv, wv;

/**
 * ADC transfer callback implementation.
 *
 * This function is being called by the ADC driver from the DMA transfer/half
 * transfer complete interrupt.
 */
void adc_transfer_callback(bool transfer_complete, uint16_t *raw_data)
{
	if (transfer_complete) {
		OFF(LED_RED);
		/* Simple IIR filter on the three phase voltage measurements. */
		uv = ((uv << 2) + raw_data[ADC_RAW_A1_UV1]
				+ raw_data[ADC_RAW_A2_UV1])/6;
		vv = ((vv << 2) + raw_data[ADC_RAW_A1_VV1]
				+ raw_data[ADC_RAW_A2_VV1])/6;
		wv = ((wv << 2) + raw_data[ADC_RAW_A1_VV1]
				+ raw_data[ADC_RAW_A2_VV1])/6;
	} else {
		ON(LED_RED);
		/* Simple IIR filter on the three phase voltage measurements. */
		uv = ((uv << 2) + raw_data[ADC_RAW_A1_UV2]
				+ raw_data[ADC_RAW_A2_UV2])/6;
		vv = ((vv << 2) + raw_data[ADC_RAW_A1_VV2]
				+ raw_data[ADC_RAW_A2_VV2])/6;
		wv = ((wv << 2) + raw_data[ADC_RAW_A1_VV2]
				+ raw_data[ADC_RAW_A2_VV2])/6;
	}
}

/**
 * ADC test main function
 *
 * @return Nothing really...
 */
int main(void)
{
	mcu_init();
	led_init();
	adc_init(adc_transfer_callback, adc_transfer_callback);
	pwm_init(); /* Initializing pwm to make sure the phases are floating. */

	uv = 0;
	vv = 0;
	wv = 0;

	while (true) {
		/* Switch on green led if the voltage on phase U exceeds 1000
		 * counts.
		 */

		if (uv > 1000) {
			ON(LED_GREEN);
		} else {
			OFF(LED_GREEN);
		}
	}
}
