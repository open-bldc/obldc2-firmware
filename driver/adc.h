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

#ifndef __ADC_H
#define __ADC_H

#include <stdint.h>

/* Definitions of the raw_data sample slots. */
#define ADC_RAW_A1_UV1 0
#define ADC_RAW_A2_VV1 1
#define ADC_RAW_A1_VV1 2
#define ADC_RAW_A2_WV1 3
#define ADC_RAW_A1_WV1 4
#define ADC_RAW_A2_UV1 5
#define ADC_RAW_A1_VB1 6
#define ADC_RAW_A2_CU1 7

#define ADC_RAW_A1_VV2 8
#define ADC_RAW_A2_UV2 9
#define ADC_RAW_A1_WV2 10
#define ADC_RAW_A2_VV2 11
#define ADC_RAW_A1_UV2 12
#define ADC_RAW_A2_WV2 13
#define ADC_RAW_A1_VB2 14
#define ADC_RAW_A2_CU2 15

typedef void (*adc_callback_t)(bool transfer_complete, uint16_t *raw_data);

void adc_init(adc_callback_t half_transfer_callback,
	      adc_callback_t transfer_complete_callback);

#endif /* __ADC_H */
