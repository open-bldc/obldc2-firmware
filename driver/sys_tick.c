/*
 * Open-BLDC - Open BrushLess DC Motor Controller
 * Copyright (C) 2010-2013 by Piotr Esden-Tempski <piotr@esden.net>
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
 * @file   sys_tick.c
 * @author Piotr Esden-Tempski <piotr@esden.net>
 *
 * @brief  Sys Tick based soft timer driver implementation
 *
 * Sys Tick is a part of the Cortex M3 core and can be used as a system timer.
 * This implementation uses it as a coarce soft timer source.
 */

#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#include <libopencm3/cm3/systick.h>

#include "driver/sys_tick.h"

#include "driver/led.h"

/**
 * Amount of available Sys Tick based soft timer slots.
 *
 * @todo move to global config header
 */
#define SYS_TICK_TIMER_NUM 5

/**
 * Private global sys tick counter.
 */
static uint32_t sys_tick_global_counter = 0;

/**
 * Represents one Sys Tick based soft timer.
 */
struct sys_tick_timer {
    sys_tick_timer_callback_t callback; /**< Callback fn-pointer */
    uint32_t start_time; /**< Start timestamp of the timer */
    uint32_t delta_time; /**< Duration of the timer */
};

/**
 * Instances of available Sys Tick timer slots.
 */
static struct sys_tick_timer sys_tick_timers[SYS_TICK_TIMER_NUM];

/**
 * Initialize Sys Tick peripheral and the soft timer slots.
 */
void sys_tick_init(void) {
    int i;

    /* Setup SysTick Timer for 100uSec Interrupts */
    systick_set_clocksource(STK_CTRL_CLKSOURCE_AHB);
    systick_set_reload((64000000 / 10000) - 1);
    systick_interrupt_enable();

    for (i = 0; i < SYS_TICK_TIMER_NUM; i++) {
        sys_tick_timers[i].callback = NULL;
        sys_tick_timers[i].start_time = 0;
        sys_tick_timers[i].delta_time = 0;
    }

    /* Start counting. */
    systick_counter_enable();
}

/**
 * Get a new timer.
 *
 * Use this if you want busy wait for some time. To detect the new time use
 * @ref sys_tick_check_timer()
 *
 * @return Timer ID
 */
uint32_t sys_tick_get_timer(void) {
    return sys_tick_global_counter;
}

/**
 * Check actively if a certain time elapsed.
 *
 * @param timer Timer aquired using @ref sys_tick_get_timer()
 * @param time Time delay to check against.
 *
 * @return 0 if the time did not elapse yet, 1 if the time elapsed.
 */
bool sys_tick_check_timer(uint32_t timer, uint32_t time) {
    if ((sys_tick_global_counter - timer) > time) {
        return true;
    } else {
        return false;
    }
}

/**
 * Register a soft timer callback.
 *
 * @param callback Callback function that should be called after a time elapses.
 * @param time Delay to wait for.
 *
 * @return ID of the soft timer, or -1 if no slots available.
 */
int sys_tick_timer_register(sys_tick_timer_callback_t callback, uint32_t time) {
    int i;
    uint32_t start_time = sys_tick_global_counter;

    for (i = 0; i < SYS_TICK_TIMER_NUM; i++) {
        if (!sys_tick_timers[i].callback) {
            sys_tick_timers[i].callback = callback;
            sys_tick_timers[i].start_time = start_time;
            sys_tick_timers[i].delta_time = time;
            return i;
        }
    }

    return -1;
}

/**
 * Unregister a soft timer.
 */
void sys_tick_timer_unregister(int id) {
    sys_tick_timers[id].callback = NULL;
    sys_tick_timers[id].start_time = 0;
    sys_tick_timers[id].delta_time = 0;
}

/**
 * Update the time for a soft timer.
 */
void sys_tick_timer_update(int id, uint32_t time) {
    sys_tick_timers[id].start_time = sys_tick_global_counter;
    sys_tick_timers[id].delta_time = time;
}

/**
 * Sys Tick interrupt handler.
 */
void sys_tick_handler(void) {
    int i;

    sys_tick_global_counter++;

    for (i = 0; i < SYS_TICK_TIMER_NUM; i++) {
        if ((sys_tick_timers[i].callback != NULL) &&
            sys_tick_check_timer(sys_tick_timers[i].start_time,
                     sys_tick_timers[i].delta_time)) {
            sys_tick_timers[i].start_time = sys_tick_global_counter;
            sys_tick_timers[i].callback(i);
        }
    }
}
