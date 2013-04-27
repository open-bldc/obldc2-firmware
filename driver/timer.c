/*
 * Open-BLDC - Open BrushLess DC Motor Controller
 * Copyright (C) 2009 by Piotr Esden-Tempski <piotr@esden.net>
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
 * @file   timer.c
 * @author Piotr Esden-Tempski <piotr@esden.net>
 * @date   Tue Aug 17 01:57:56 2010
 *
 * @brief  Timer subsystem global implementation
 *
 */

#include <stdint.h>
#include <unistd.h>

#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/nvic.h>
#include <libopencm3/stm32/timer.h>

#include "driver/timer.h"

#include "driver/led.h"

/* Set timer input frequency. (resolution .25us) */
#define TIMER_FREQUENCY 4000000

struct timer_entry {
    volatile uint16_t next_invocation;
    volatile uint16_t delta_ticks;
    timer_callback_t callback;
};

/* Internal state. */
struct timer_state {
    struct timer_entry entry[4];
} timer_state;

/**
 * Initialize the three phase (6outputs) PWM peripheral and internal state.
 */
void timer_init(void)
{
    /* Initialize state. */
    for (int i = 0; i < 4; i++) {
        timer_state.entry[i].next_invocation = 0;
        timer_state.entry[i].delta_ticks = 0;
        timer_state.entry[i].callback = NULL;
    }

    /* Enable clock for TIM subsystem */
    rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_TIM2EN);

    /* Enable TIM interrupt */
    nvic_enable_irq(NVIC_TIM2_IRQ);

    /* Reset TIM peripheral */
    timer_reset(TIM2);

    /* Timer global mode:
     * - No divider
     * - alignment center
     *   We don't care if it is CENTER 1,2 or 3 as this influences only
     *   interrupt generation and we don't generate interrupts based on
     *   the pwm signal.
     * - direction up
     */
    timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT,
                   TIM_CR1_CMS_EDGE,
                   TIM_CR1_DIR_UP);

    timer_set_prescaler(TIM2, (64000000 / TIMER_FREQUENCY) - 1);
    timer_enable_preload(TIM2);
    timer_continuous_mode(TIM2);
    timer_set_period(TIM2, UINT16_MAX);

    /* -- OC1 configuration -- */

    /* Disable outputs. */
    timer_disable_oc_output(TIM2, TIM_OC1);

    /* Configure global mode. */
    timer_disable_oc_clear(TIM2, TIM_OC1);
    timer_disable_oc_preload(TIM2, TIM_OC1);
    timer_set_oc_slow_mode(TIM2, TIM_OC1);
    timer_set_oc_mode(TIM2, TIM_OC1, TIM_OCM_FROZEN);

    /* Set the capture compare value for OC.
     * NOTE: This does not matter much here as we are not enabling
     * interrupts yet...
     */
    timer_set_oc_value(TIM2, TIM_OC1, 0);

    /* -- OC2 configuration -- */

    /* Disable outputs. */
    timer_disable_oc_output(TIM2, TIM_OC2);

    /* Configure global mode of line 1. */
    timer_disable_oc_clear(TIM2, TIM_OC2);
    timer_disable_oc_preload(TIM2, TIM_OC2);
    timer_set_oc_slow_mode(TIM2, TIM_OC2);
    timer_set_oc_mode(TIM2, TIM_OC2, TIM_OCM_FROZEN);

    /* Set the capture compare value for OC.
     * NOTE: This does not matter much here as we are not enabling
     * interrupts yet...
     */
    timer_set_oc_value(TIM2, TIM_OC2, 0);

    /* -- OC3 configuration -- */

    /* Disable outputs. */
    timer_disable_oc_output(TIM2, TIM_OC3);

    /* Configure global mode. */
    timer_disable_oc_clear(TIM2, TIM_OC3);
    timer_disable_oc_preload(TIM2, TIM_OC3);
    timer_set_oc_slow_mode(TIM2, TIM_OC3);
    timer_set_oc_mode(TIM2, TIM_OC3, TIM_OCM_FROZEN);

    /* Set the capture compare value for OC.
     * NOTE: This does not matter much here as we are not enabling
     * interrupts yet...
     */
    timer_set_oc_value(TIM2, TIM_OC3, 0);

    /* -- OC4 configuration -- */

    /* Disable outputs. */
    timer_disable_oc_output(TIM2, TIM_OC4);

    /* Configure global mode. */
    timer_disable_oc_clear(TIM2, TIM_OC4);
    timer_disable_oc_preload(TIM2, TIM_OC4);
    timer_set_oc_slow_mode(TIM2, TIM_OC4);
    timer_set_oc_mode(TIM2, TIM_OC4, TIM_OCM_FROZEN);

    /* Set the capture compare value for OC.
     * NOTE: This does not matter much here as we are not enabling
     * interrupts yet...
     */
    timer_set_oc_value(TIM2, TIM_OC4, 0);

    /* ---- */
    /* ARR reload enable */
    timer_enable_preload(TIM2);

    /* Counter enable */
    timer_enable_counter(TIM2);

    /* Disable compare interrupts. */
    timer_disable_irq(TIM2, TIM_DIER_CC1IE |
                            TIM_DIER_CC2IE |
                            TIM_DIER_CC3IE |
                            TIM_DIER_CC4IE);

}

int timer_register(uint16_t delta_ticks, timer_callback_t callback) {
    int timer_id = -1;
    uint16_t now = timer_get_counter(TIM2);

    /* Too short delay, this will result in an interrupt storm? */
    if (delta_ticks < 4)
        return -2;

    for (int i = 0; i < 4; i++) {
        if(!timer_state.entry[i].callback) {
            timer_state.entry[i].callback = callback;
            timer_state.entry[i].next_invocation = now + delta_ticks;
            timer_state.entry[i].delta_ticks = delta_ticks;
            timer_set_oc_value(TIM2, i * 2, now + delta_ticks);
            timer_enable_irq(TIM2, 1 << (i + 1));
            timer_id = i;
            break;
        }
    }

    return timer_id;
}

void timer_unregister(int timer_id) {
    if (timer_id < 0 || timer_id > 3)
        return;

    timer_state.entry[timer_id].callback = NULL;
    timer_disable_irq(TIM2, 1 << (timer_id+1));
}

void timer_modify_delta(int timer_id, uint16_t delta_ticks) {
    timer_state.entry[timer_id].delta_ticks = delta_ticks;
}

/**
 * Timer event interrupt handler
 */
void tim2_isr(void)
{
    if (timer_get_flag(TIM2, TIM_SR_CC1IF)) {
        timer_clear_flag(TIM2, TIM_SR_CC1IF);

        /* Run callback if available. */
        if (timer_state.entry[0].callback)
            timer_state.entry[0].callback(0, timer_state.entry[0].next_invocation);
        else
            timer_disable_irq(TIM2, TIM_DIER_CC1IE);

        /* Update last invocation time. */
        timer_state.entry[0].next_invocation += timer_state.entry[0].delta_ticks;

        timer_set_oc_value(TIM2, TIM_OC1, timer_state.entry[0].next_invocation);
    }

    if (timer_get_flag(TIM2, TIM_SR_CC2IF)) {
        timer_clear_flag(TIM2, TIM_SR_CC2IF);

        /* Run callback if available. */
        if (timer_state.entry[1].callback)
            timer_state.entry[1].callback(1, timer_state.entry[1].next_invocation);
        else
            timer_disable_irq(TIM2, TIM_DIER_CC2IE);

        /* Update last invocation time. */
        timer_state.entry[1].next_invocation += timer_state.entry[1].delta_ticks;

        timer_set_oc_value(TIM2, TIM_OC2, timer_state.entry[1].next_invocation);
    }

    if (timer_get_flag(TIM2, TIM_SR_CC3IF)) {
        timer_clear_flag(TIM2, TIM_SR_CC3IF);

        /* Run callback if available. */
        if (timer_state.entry[2].callback)
            timer_state.entry[2].callback(2, timer_state.entry[2].next_invocation);
        else
            timer_disable_irq(TIM2, TIM_DIER_CC3IE);

        /* Update last invocation time. */
        timer_state.entry[2].next_invocation += timer_state.entry[2].delta_ticks;

        timer_set_oc_value(TIM2, TIM_OC3, timer_state.entry[2].next_invocation);
    }

    if (timer_get_flag(TIM2, TIM_SR_CC4IF)) {
        timer_clear_flag(TIM2, TIM_SR_CC4IF);

        /* Run callback if available. */
        if (timer_state.entry[3].callback)
            timer_state.entry[3].callback(3, timer_state.entry[3].next_invocation);
        else
            timer_disable_irq(TIM2, TIM_DIER_CC4IE);

        /* Update last invocation time. */
        timer_state.entry[3].next_invocation += timer_state.entry[3].delta_ticks;

        timer_set_oc_value(TIM2, TIM_OC4, timer_state.entry[3].next_invocation);
    }
}