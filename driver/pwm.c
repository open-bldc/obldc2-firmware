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
 * @file   pwm.c
 * @author Piotr Esden-Tempski <piotr@esden.net>
 * @date   Tue Aug 17 01:57:56 2010
 *
 * @brief  PWM subsystem global implementation
 *
 * This is the umbrella above all the PWM schemes.
 */

#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/nvic.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/f1/gpio.h>

#include <lg/gpdef.h>
#include <lg/gprotc.h>

#include "driver/pwm.h"

#include "driver/led.h"

/* PWM_DEFINES */
#define PWM__ZERO_VALUE 0x3FF

/* At 64MHz system clock this should result in about 16KHz pwm frequency.
 * Thanks to the pwm scheme we are running the motor will see double the
 * frequency, resulting in ~32khz!
 *
 * As we are using center aligned PWM the period has to be half as big.
 * As we need two (one up count and one downcount) period counts per
 * waveform periods.
 *
 * Using 2047 divider. (64MHz / 0x7FF = 15.632KHz)
 */
#define PWM__MAX_VALUE 0x7FF

/* Internal state. */

struct pwm_state {
    volatile bool on;
    volatile int step;
    volatile bool idle;
    volatile int16_t value;
} pwm_state;

/**
 * Initialize the three phase (6outputs) PWM peripheral and internal state.
 */
void pwm_init(void)
{
    /* Initialize state. */
    pwm_state.on = false;
    pwm_state.idle = true;
    pwm_state.step = 0;

    /* Enable clock for TIM1 subsystem */
    rcc_peripheral_enable_clock(&RCC_APB2ENR,
                                RCC_APB2ENR_TIM1EN |
                                RCC_APB2ENR_IOPAEN |
                                RCC_APB2ENR_IOPBEN |
                                RCC_APB2ENR_AFIOEN);

    /* GPIOA: TIM1 channel 1, 2 and 3 as alternate function
       push-pull */
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  GPIO_TIM1_CH1 |
                  GPIO_TIM1_CH2 |
                  GPIO_TIM1_CH3);

    /* GPIOB: TIM1 channel 1N, 2N and 3N as alternate function
     * push-pull
     */
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                  GPIO_TIM1_CH1N |
                  GPIO_TIM1_CH2N |
                  GPIO_TIM1_CH3N);

    /* Enable TIM1 commutation interrupt */
    nvic_enable_irq(NVIC_TIM1_TRG_COM_IRQ);

    /* Enable TIM1 capture/compare interrupt */
    nvic_enable_irq(NVIC_TIM1_CC_IRQ);

    /* Reset TIM1 peripheral */
    timer_reset(TIM1);

    /* Timer global mode:
     * - No divider
     * - alignment center
     *   We don't care if it is CENTER 1,2 or 3 as this influences only
     *   interrupt generation and we don't generate interrupts based on
     *   the pwm signal.
     * - direction up
     */
    timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT,
                   TIM_CR1_CMS_CENTER_1,
                   TIM_CR1_DIR_UP);

    /* Reset prescaler value. */
    timer_set_prescaler(TIM1, 0);

    /* Reset repetition counter value. */
    timer_set_repetition_counter(TIM1, 0);

    /* Enable preload. */
    timer_enable_preload(TIM1);

    /* Continous mode. */
    timer_continuous_mode(TIM1);

    /* Period (PWM__FREQUENCY) */
    timer_set_period(TIM1, PWM__MAX_VALUE);

    /* Configure break and deadtime */
    timer_set_deadtime(TIM1, 0);
    timer_set_enabled_off_state_in_idle_mode(TIM1);
    timer_set_enabled_off_state_in_run_mode(TIM1);
    timer_disable_break(TIM1);
    timer_set_break_polarity_high(TIM1);
    timer_disable_break_automatic_output(TIM1);
    timer_set_break_lock(TIM1, TIM_BDTR_LOCK_OFF);

    /* -- OC1 and OC1N configuration -- */

    /* Disable outputs. */
    timer_disable_oc_output(TIM1, TIM_OC1);
    timer_disable_oc_output(TIM1, TIM_OC1N);

    /* Configure global mode of line 1. */
    timer_disable_oc_clear(TIM1, TIM_OC1);
    timer_enable_oc_preload(TIM1, TIM_OC1);
    timer_set_oc_slow_mode(TIM1, TIM_OC1);
    timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_FORCE_LOW);

    /* Configure OC1. */
    timer_set_oc_polarity_high(TIM1, TIM_OC1);
    timer_set_oc_idle_state_set(TIM1, TIM_OC1);

    /* Configure OC1N. */
    timer_set_oc_polarity_low(TIM1, TIM_OC1N);
    timer_set_oc_idle_state_set(TIM1, TIM_OC1N);

    /* Set the capture compare value for OC1. */
    timer_set_oc_value(TIM1, TIM_OC1, PWM__ZERO_VALUE);

    /* Enable outputs. */
    timer_enable_oc_output(TIM1, TIM_OC1);
    timer_enable_oc_output(TIM1, TIM_OC1N);

    /* -- OC2 and OC2N configuration -- */

    /* Disable outputs. */
    timer_disable_oc_output(TIM1, TIM_OC2);
    timer_disable_oc_output(TIM1, TIM_OC2N);

    /* Configure global mode of line 2. */
    timer_disable_oc_clear(TIM1, TIM_OC2);
    timer_enable_oc_preload(TIM1, TIM_OC2);
    timer_set_oc_slow_mode(TIM1, TIM_OC2);
    timer_set_oc_mode(TIM1, TIM_OC2, TIM_OCM_FORCE_LOW);

    /* Configure OC2. */
    timer_set_oc_polarity_high(TIM1, TIM_OC2);
    timer_set_oc_idle_state_set(TIM1, TIM_OC2);

    /* Configure OC2N. */
    timer_set_oc_polarity_low(TIM1, TIM_OC2N);
    timer_set_oc_idle_state_set(TIM1, TIM_OC2N);

    /* Set the capture compare value for OC1. */
    timer_set_oc_value(TIM1, TIM_OC2, PWM__ZERO_VALUE);

    /* Enable outputs. */
    timer_enable_oc_output(TIM1, TIM_OC2);
    timer_enable_oc_output(TIM1, TIM_OC2N);

    /* -- OC3 and OC3N configuration -- */

    /* Disable outputs. */
    timer_disable_oc_output(TIM1, TIM_OC3);
    timer_disable_oc_output(TIM1, TIM_OC3N);

    /* Configure global mode of line 3. */
    timer_disable_oc_clear(TIM1, TIM_OC3);
    timer_enable_oc_preload(TIM1, TIM_OC3);
    timer_set_oc_slow_mode(TIM1, TIM_OC3);
    timer_set_oc_mode(TIM1, TIM_OC3, TIM_OCM_FORCE_LOW);

    /* Configure OC3. */
    timer_set_oc_polarity_high(TIM1, TIM_OC3);
    timer_set_oc_idle_state_set(TIM1, TIM_OC3);

    /* Configure OC3N. */
    timer_set_oc_polarity_low(TIM1, TIM_OC3N);
    timer_set_oc_idle_state_set(TIM1, TIM_OC3N);

    /* Set the capture compare value for OC3. */
    timer_set_oc_value(TIM1, TIM_OC3, PWM__ZERO_VALUE);

    /* Enable outputs. */
    timer_enable_oc_output(TIM1, TIM_OC3);
    timer_enable_oc_output(TIM1, TIM_OC3N);

    /* ---- */
    /* ARR reload enable */
    timer_enable_preload(TIM1);

    /* Enable preload of complementary channel configurations and update on COM event */
    timer_enable_preload_complementry_enable_bits(TIM1);

    /* Enable outputs in the break subsystem */
    timer_enable_break_main_output(TIM1);

    /* Counter enable */
    timer_enable_counter(TIM1);

    /* Enable commutation interrupt */
    timer_enable_irq(TIM1, TIM_DIER_COMIE);

}

/**
 * Trigger one commutation event.
 */
void pwm_comm(void)
{
    timer_generate_event(TIM1, TIM_EGR_COMG);
}

/**
 * Set all half bridges to floating. (High side and low side mosfets off)
 */
void pwm_off(void)
{
    timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_FORCE_LOW);
    timer_set_oc_mode(TIM1, TIM_OC2, TIM_OCM_FORCE_LOW);
    timer_set_oc_mode(TIM1, TIM_OC3, TIM_OCM_FORCE_LOW);
    timer_enable_oc_output(TIM1, TIM_OC1);
    timer_enable_oc_output(TIM1, TIM_OC1N);
    timer_enable_oc_output(TIM1, TIM_OC2);
    timer_enable_oc_output(TIM1, TIM_OC2N);
    timer_enable_oc_output(TIM1, TIM_OC3);
    timer_enable_oc_output(TIM1, TIM_OC3N);
    pwm_comm();
}

/**
 * Switch on the low side only. (Switching only the high side on is silly as
 * the bootstraps won't be able to keep the high side mosfets on for very long
 * anyways)
 */
void pwm_all_lo(void)
{
    timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_FORCE_HIGH);
    timer_set_oc_mode(TIM1, TIM_OC2, TIM_OCM_FORCE_HIGH);
    timer_set_oc_mode(TIM1, TIM_OC3, TIM_OCM_FORCE_HIGH);
    timer_disable_oc_output(TIM1, TIM_OC1);
    timer_enable_oc_output(TIM1, TIM_OC1N);
    timer_disable_oc_output(TIM1, TIM_OC2);
    timer_enable_oc_output(TIM1, TIM_OC2N);
    timer_disable_oc_output(TIM1, TIM_OC3);
    timer_enable_oc_output(TIM1, TIM_OC3N);
    pwm_comm();
}

/**
 * Set the pwm duty cycle according to the current comm state.
 */
void pwm_set(int16_t value) {
    /* Store the value passet into the driver state. */
    pwm_state.value = value;

    /* Divide the value passed by 2^5, as this is our pwm range available. */
    value /= 1<<5;

    /* Calculate and set the pwm values for the phases.
     * See that we are setting the pwm value for the disabled phase too.
     * This is in advance of a commutation. This way me make sure that the pwm
     * value is definitely set.
     */
    switch (pwm_state.step) {
        case 0:
            timer_set_oc_value(TIM1, TIM_OC1, PWM__ZERO_VALUE - value); /* enabled is low */
            timer_set_oc_value(TIM1, TIM_OC2, PWM__ZERO_VALUE - value); /* disabled will be low */
            timer_set_oc_value(TIM1, TIM_OC3, PWM__ZERO_VALUE + value); /* enabled is high */
            break;
        case 1:
            timer_set_oc_value(TIM1, TIM_OC1, PWM__ZERO_VALUE + value); /* disabled will be high */
            timer_set_oc_value(TIM1, TIM_OC2, PWM__ZERO_VALUE - value); /* enabled is low */
            timer_set_oc_value(TIM1, TIM_OC3, PWM__ZERO_VALUE + value); /* enabled is high */
            break;
        case 2:
            timer_set_oc_value(TIM1, TIM_OC1, PWM__ZERO_VALUE + value); /* enabled is high */
            timer_set_oc_value(TIM1, TIM_OC2, PWM__ZERO_VALUE - value); /* enabled is low */
            timer_set_oc_value(TIM1, TIM_OC3, PWM__ZERO_VALUE - value); /* disabled will be low */
            break;
        case 3:
            timer_set_oc_value(TIM1, TIM_OC1, PWM__ZERO_VALUE + value); /* enabled is high */
            timer_set_oc_value(TIM1, TIM_OC2, PWM__ZERO_VALUE + value); /* disabled will be high */
            timer_set_oc_value(TIM1, TIM_OC3, PWM__ZERO_VALUE - value); /* enabled is low */
            break;
        case 4:
            timer_set_oc_value(TIM1, TIM_OC1, PWM__ZERO_VALUE - value); /* disabled will be low */
            timer_set_oc_value(TIM1, TIM_OC2, PWM__ZERO_VALUE + value); /* enabled is high */
            timer_set_oc_value(TIM1, TIM_OC3, PWM__ZERO_VALUE - value); /* enabled is low */
            break;
        case 5:
            timer_set_oc_value(TIM1, TIM_OC1, PWM__ZERO_VALUE - value); /* enabled is low */
            timer_set_oc_value(TIM1, TIM_OC2, PWM__ZERO_VALUE + value); /* enabled is high */
            timer_set_oc_value(TIM1, TIM_OC3, PWM__ZERO_VALUE + value); /* disabled will be high */
            break;
    }
}

/**
 * PWM timer commutation event interrupt handler
 */
void tim1_trg_com_isr(void)
{
    timer_clear_flag(TIM1, TIM_SR_COMIF);

    TOGGLE(LED_GREEN);

    /* TODO: 
     * We should check if the comm event actually enabled a pwm. And then set
     * the pwm_state.on value accordingly.
     */

    if (pwm_state.idle) {
        pwm_state.idle = false;
        switch (pwm_state.step) {
        case 0:         // 000º
        case 3:         // 180º
            pwm_state.step++;
            /* Configure step 1 or 4 */
            timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM1);
            timer_set_oc_mode(TIM1, TIM_OC2, TIM_OCM_PWM1);
            timer_set_oc_mode(TIM1, TIM_OC3, TIM_OCM_FORCE_LOW);
            timer_enable_oc_output(TIM1, TIM_OC1);
            timer_disable_oc_output(TIM1, TIM_OC1N);
            timer_enable_oc_output(TIM1, TIM_OC2);
            timer_disable_oc_output(TIM1, TIM_OC2N);
            timer_enable_oc_output(TIM1, TIM_OC3);
            timer_enable_oc_output(TIM1, TIM_OC3N);
            break;
        case 1:         // 060º
        case 4:         // 220º
            /* Configure step 2 or 5 */
            timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_PWM1);
            timer_set_oc_mode(TIM1, TIM_OC2, TIM_OCM_FORCE_LOW);
            timer_set_oc_mode(TIM1, TIM_OC3, TIM_OCM_PWM1);
            timer_enable_oc_output(TIM1, TIM_OC1);
            timer_disable_oc_output(TIM1, TIM_OC1N);
            timer_enable_oc_output(TIM1, TIM_OC2);
            timer_enable_oc_output(TIM1, TIM_OC2N);
            timer_enable_oc_output(TIM1, TIM_OC3);
            timer_disable_oc_output(TIM1, TIM_OC3N);
            pwm_state.step++;
            break;
        case 5:         // 280º
            pwm_state.step = -1;
        case 2:         // 120º
            pwm_state.step++;
            /* Configure step 3 or 0 */
            timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_FORCE_LOW);
            timer_set_oc_mode(TIM1, TIM_OC2, TIM_OCM_PWM1);
            timer_set_oc_mode(TIM1, TIM_OC3, TIM_OCM_PWM1);
            timer_enable_oc_output(TIM1, TIM_OC1);
            timer_enable_oc_output(TIM1, TIM_OC1N);
            timer_enable_oc_output(TIM1, TIM_OC2);
            timer_disable_oc_output(TIM1, TIM_OC2N);
            timer_enable_oc_output(TIM1, TIM_OC3);
            timer_disable_oc_output(TIM1, TIM_OC3N);
            break;
        }
        pwm_comm();
    } else {
        /* We are inserting an idle state between commutations.
         * In this state all phases are floating. This prevents
         * From pwm cycle cutoffs creating high power pulses to
         * the phases between commutations.
         *
         * A better solution would be to have the PWM waveform
         * Move continously onto the next phase without generating
         * gaps. Not sure if that is doable though.
         */
        pwm_state.idle = true;
        timer_set_oc_mode(TIM1, TIM_OC1, TIM_OCM_FORCE_LOW);
        timer_set_oc_mode(TIM1, TIM_OC2, TIM_OCM_FORCE_LOW);
        timer_set_oc_mode(TIM1, TIM_OC3, TIM_OCM_FORCE_LOW);
        timer_enable_oc_output(TIM1, TIM_OC1);
        timer_enable_oc_output(TIM1, TIM_OC1N);
        timer_enable_oc_output(TIM1, TIM_OC2);
        timer_enable_oc_output(TIM1, TIM_OC2N);
        timer_enable_oc_output(TIM1, TIM_OC3);
        timer_enable_oc_output(TIM1, TIM_OC3N);
    }

    /* Make sure that the pwm duty cycle setting is done at least once
     * a commutation.
     */
    pwm_set(pwm_state.value);

    OFF(LED_GREEN);
}