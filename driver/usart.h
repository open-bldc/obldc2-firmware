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

#ifndef __USART_H
#define __USART_H

#include <stdint.h>

typedef int (*usart_handle_byte_callback_t)(uint8_t byte);
typedef int32_t (*usart_get_byte_callback_t)(void);

void usart_init(usart_handle_byte_callback_t handle_byte_callback,
		usart_get_byte_callback_t get_byte_callback);
void usart_enable_send(void);
void usart_disable_send(void);

#endif /* __USART_H */
