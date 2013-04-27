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

#ifndef __TIMER_H
#define __TIMER_H

#include <stdint.h>

typedef void (*timer_callback_t)(int timer_id, uint16_t time);

void timer_init(void);
int timer_register(uint16_t delta_ticks, timer_callback_t callback);
void timer_unregister(int timer_id);
void timer_modify_delta(int timer_id, uint16_t delta_ticks);

#endif /* __TIMER_H */