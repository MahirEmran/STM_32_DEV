/**
* This file is part of the Titan Flight Computer Project
* Copyright (c) 2025 UW SARP
* 
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, version 3.
* 
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
* 
* @file peripheral/pwm.c
* @authors Joshua Beard, Aaron Mcbride, Jude Merritt
* @brief Implementation of the PWM driver interface
* */

#include "peripheral/pwm.h"
#include "peripheral/errc.h"
#include "peripheral/gpio.h"
#include "internal/mmio.h"


/**************************************************************************************************
* @section Macros
**************************************************************************************************/

#define MAX_DUTY_CYCLE 1000
#define PWM_INSTANCE_COUNT 8 //is this correct?


/**************************************************************************************************
* @section Private Function Implementations
**************************************************************************************************/

static enum ti_errc_t check_pwm_config_validity(struct ti_pwm_config_t pwm_config) {
    
    if (pwm_config.freq <= 0) {
        return TI_ERRC_INVALID_ARG;
    }

    int32_t freq_prescaler = PWM_CLOCK_FREQ / pwm_config.freq;
    
    if (freq_prescaler > UINT16_MAX) {
        return TI_ERRC_INVALID_ARG;
    }

    if (pwm_config.duty < 0 || pwm_config.duty > MAX_DUTY_CYCLE) {
        return TI_ERRC_INVALID_ARG;
    }
    
    // TODO: validate pin, channel, alt number

    return TI_ERRC_NONE; 
}

static inline void pwm_enable_channel(int32_t pwm_inst, uint32_t channel) {
    
    // Enable PWM channel output on the timer
    SET_FIELD(G_TIMx_CCER[pwm_inst], G_TIMx_CCER_CCxE[channel]);

    //Enable the timer
    SET_FIELD(G_TIMx_CR1[pwm_inst], G_TIMx_CR1_CEN);

    // Enable PWM output
    SET_FIELD(G_TIMx_CR1[pwm_inst], G_TIMx_CR1_ARPE);
}

static inline void pwm_setup_output_compare(int32_t pwm_inst, uint32_t channel) {
    if (channel <= 3) {
        WRITE_FIELD(G_TIMx_CCMR2_OUTPUT[pwm_inst], G_TIMx_CCMR2_OUTPUT_OCxM[channel], 0b0110); //added to line 7889 of mmio
        SET_FIELD(G_TIMx_CCMR2_OUTPUT[pwm_inst], G_TIMx_CCMR2_OUTPUT_OCxPE[channel]); //added to line 7890 of mmio
    } else {
        WRITE_FIELD(G_TIMx_CCMR1_OUTPUT[pwm_inst], G_TIMx_CCMR1_OUTPUT_OCxM[channel], 0b0110); //TODO: Replace 0b0110 with a define constant
        SET_FIELD(G_TIMx_CCMR1_OUTPUT[pwm_inst], G_TIMx_CCMR2_OUTPUT_OCxPE[channel]);
    }
}


/**************************************************************************************************
* @section Public Function Implementations
**************************************************************************************************/

//I am a function comment placeholder :)
void ti_set_pwm(int32_t pwm_inst, struct ti_pwm_config_t pwm_config, enum ti_errc_t* errc) {
    // asm("BKPT #0");
    //Check for potential errors
    *errc = TI_ERRC_NONE;
    
    if (pwm_inst >= PWM_INSTANCE_COUNT) {
        *errc = TI_ERRC_INVALID_ARG;
        return;
    }
    // asm("BKPT #0");
    enum ti_errc_t validation = check_pwm_config_validity(pwm_config); 
    // asm("BKPT #0");
    if (validation != TI_ERRC_NONE) {
        *errc = validation;
        return;
    }
    // asm("BKPT #0");

    if (pwm_config.freq == 0 || pwm_config.duty == 0) {
        CLR_FIELD(RCC_APB1LENR, RCC_APB1LENR_TIMxEN[pwm_inst]);
        *errc = TI_ERRC_INVALID_ARG;
        return;  
    }

    //Enable PWM clock
    SET_FIELD(RCC_APB1LENR, RCC_APB1LENR_TIMxEN[pwm_inst]);
    // asm("BKPT #0");

    //Set up GPIO pin
    tal_enable_clock(pwm_config.pin);
    tal_alternate_mode(pwm_config.pin, pwm_config.alt_num);
    tal_set_mode(pwm_config.pin, pwm_config.alt_num);

    //Set frequency of timer
    int32_t freq_prescaler = PWM_CLOCK_FREQ / pwm_config.freq;
    // asm("BKPT #0");
    WRITE_FIELD(G_TIMx_ARR[pwm_inst], G_TIMx_ARR_ARR_L, freq_prescaler);
    // asm("BKPT #0");
    // Set duty cycle
    WRITE_FIELD(G_TIMx_CCR3[pwm_inst], G_TIMx_CCR3_CCR3_L, (freq_prescaler / MAX_DUTY_CYCLE) * pwm_config.duty); //TODO: Double check math for third parameter
    // asm("BKPT #0");
    // Set to output compare
    pwm_setup_output_compare(pwm_inst, pwm_config.channel);
    // asm("BKPT #0");
    //Enable the PWM channel
    pwm_enable_channel(pwm_inst, pwm_config.channel);
    // asm("BKPT #0");
}