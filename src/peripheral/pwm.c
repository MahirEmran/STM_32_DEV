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

// #define PWM_CLOCK_FREQ 2000000
#define MAX_DUTY_CYCLE 1000
#define INSTANCE_COUNT 8 


/**************************************************************************************************
* @section Private Function Implementations
**************************************************************************************************/

static enum ti_errc_t check_pwm_config_validity(struct ti_pwm_config_t pwm_config) {
    
    if (pwm_config.freq <= 0) {
        return TI_ERRC_INVALID_ARG;
    }

    int32_t freq_prescaler = pwm_config.clock_freq / pwm_config.freq;
    // asm("BKPT #0");
    if (freq_prescaler > UINT16_MAX) {
        // asm("BKPT #0");
        return TI_ERRC_INVALID_ARG;
    }
    // asm("BKPT #0");

    if (pwm_config.duty < 0 || pwm_config.duty > MAX_DUTY_CYCLE) {
        // asm("BKPT #0");
        return TI_ERRC_INVALID_ARG;
    }
    // asm("BKPT #0");
    
    // TODO: validate pin, channel, alt number

    return TI_ERRC_NONE; 
}

static inline void pwm_enable_channel(int32_t instance, uint32_t channel) {
    
    // Enable PWM channel output on the timer
    SET_FIELD(G_TIMx_CCER[instance], G_TIMx_CCER_CCxE[channel]);
    // asm("BKPT #0");
    //Enable the timer
    SET_FIELD(G_TIMx_CR1[instance], G_TIMx_CR1_CEN);
    // asm("BKPT #0");
    // Enable PWM output
    SET_FIELD(G_TIMx_CR1[instance], G_TIMx_CR1_ARPE);
    
    // asm("BKPT #0");
}

// static inline void pwm_setup_output_compare(int32_t pwm_config.instance, uint32_t channel) {
//     if (channel <= 3) {
//         WRITE_FIELD(G_TIMx_CCMR2_OUTPUT[pwm_config.instance], G_TIMx_CCMR2_OUTPUT_OCxM[channel], 0b0110); //added to line 7889 of mmio
//         SET_FIELD(G_TIMx_CCMR2_OUTPUT[pwm_config.instance], G_TIMx_CCMR2_OUTPUT_OCxPE[channel]); //added to line 7890 of mmio
//     } else {
//         WRITE_FIELD(G_TIMx_CCMR1_OUTPUT[pwm_config.instance], G_TIMx_CCMR1_OUTPUT_OCxM[channel], 0b0110); //TODO: Replace 0b0110 with a define constant
//         SET_FIELD(G_TIMx_CCMR1_OUTPUT[pwm_config.instance], G_TIMx_CCMR2_OUTPUT_OCxPE[channel]);
//     }
// }

static inline void pwm_setup_output_compare(uint32_t instance, uint32_t channel) { 
    // Use a defined constant instead of 0b0110 (PWM Mode 1)
    // TODO: is this PWM mode 1 right? check it
    #define PWM_MODE1 0b0110
    // asm("BKPT #0");
    if (channel == 1 || channel == 2) {
        WRITE_FIELD(G_TIMx_CCMR1_OUTPUT[instance], G_TIMx_CCMR1_OUTPUT_OCxM[channel], PWM_MODE1);
        SET_FIELD(G_TIMx_CCMR1_OUTPUT[instance], G_TIMx_CCMR1_OUTPUT_OCxPE[channel]); 
        // asm("BKPT #0");
    } else if (channel == 3 || channel == 4) { 
        WRITE_FIELD(G_TIMx_CCMR2_OUTPUT[instance], G_TIMx_CCMR2_OUTPUT_OCxM[channel], PWM_MODE1); 
        SET_FIELD(G_TIMx_CCMR2_OUTPUT[instance], G_TIMx_CCMR2_OUTPUT_OCxPE[channel]);
        // asm("BKPT #0");
    }
}

void pwm_set_pin_vals(int* pin, int* alt_mode, int32_t instance, int32_t channel) {
    *alt_mode =  instance == 2 ? 1 : 2;
    switch (instance) {
        // case 1:
        //     switch (channel) {
        //         case 1:
        //             *pin = 59; // E9
        //             break;
        //         case 2:
        //             *pin = 61; // E11
        //             break;
        //         case 3:
        //             *pin = 63; // E13
        //             break;
        //         case 4:
        //             *pin = 64; // E14
        //             break;
        //         default:
        //             break;
        //     }
        //     break;
        case 2:
            switch (channel) {
                case 1:
                    // *pin = 37; // A0
                    // *pin = 44; // A5
                    *pin = 108; // A15
                    break;
                case 2:
                    // *pin = 38; // A1
                    *pin = 130; // B3
                    break;
                case 3:
                    // *pin = 39; // A2
                    *pin = 66; // B10
                    break;
                case 4:
                    // *pin = 40; // A3
                    *pin = 67; // B11
                    break;
                default:
                    break;
            }
            break;
        case 3:
            switch (channel) {
                case 1:
                    *pin = 45; // A6
                    // *pin = 93; // C6
                    break;
                case 2:
                    *pin = 46; // A7
                    // *pin = 94; // C7
                    break;
                case 3:
                    *pin = 49; // B0
                    // *pin = 95; // C8
                    break;
                case 4:
                    *pin = 50; // B1
                    // *pin = 96; // C9
                    break;
                default:
                    break;
            }
            break;
        case 4:
            switch (channel) {
                case 1:
                    *pin = 133; // B6
                    break;
                case 2:
                    *pin = 134; // B7
                    break;
                case 3:
                    *pin = 136; // B8
                    break;
                case 4:
                    *pin = 137; // B9
                    break;
                default:
                    break;
            }
            break;
        case 5:
          switch (channel) {
                case 1:
                    *pin = 37; // A0
                    break;
                case 2:
                    *pin = 38; // A1
                    break;
                case 3:
                    *pin = 39; // A2
                    break;
                case 4:
                    *pin = 40; // A3
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    
}


/**************************************************************************************************
* @section Public Function Implementations
**************************************************************************************************/


//I am a function comment placeholder :)
void ti_set_pwm(struct ti_pwm_config_t pwm_config, enum ti_errc_t* errc) {
    // asm("BKPT #0");
    //Check for potential errors
    *errc = TI_ERRC_NONE;
    
    if (pwm_config.instance >= INSTANCE_COUNT) {
        *errc = TI_ERRC_INVALID_ARG;
        return;
    }
    // asm("BKPT #0");
    enum ti_errc_t validation = check_pwm_config_validity(pwm_config); 
    // asm("BKPT #0");
    // asm("BKPT #0");
    if (validation != TI_ERRC_NONE) {
        *errc = validation;
        return;
    }
    // asm("BKPT #0");

    if (pwm_config.freq == 0 || pwm_config.duty == 0) {
        CLR_FIELD(RCC_APB1LENR, RCC_APB1LENR_TIMxEN[pwm_config.instance]);
        *errc = TI_ERRC_INVALID_ARG;
        return;  
    }

    //Enable PWM clock
    SET_FIELD(RCC_APB1LENR, RCC_APB1LENR_TIMxEN[pwm_config.instance]);
    // asm("BKPT #0");

    //Set up GPIO pin
    // TODO: helper method to determine pins/alt mode
    int pin;
    int alt_mode;
    pwm_set_pin_vals(&pin, &alt_mode, pwm_config.instance, pwm_config.channel);
    // asm("BKPT #0");
    tal_enable_clock(pin);
    tal_set_mode(pin, 2);
    tal_alternate_mode(pin, alt_mode);

    //Set frequency of timer
    int32_t freq_prescaler = (pwm_config.clock_freq / pwm_config.freq) - 1;

    // asm("BKPT #0");
    WRITE_FIELD(G_TIMx_ARR[pwm_config.instance], G_TIMx_ARR_ARR_L, freq_prescaler);
    // asm("BKPT #0");
    // Set duty cycle
    int32_t ccr_value = (freq_prescaler * pwm_config.duty) / MAX_DUTY_CYCLE;
    switch(pwm_config.channel) {
        case 1: 
            WRITE_FIELD(G_TIMx_CCR1[pwm_config.instance], G_TIMx_CCR1_CCR1_L, ccr_value);
            break;
        case 2: 
            WRITE_FIELD(G_TIMx_CCR2[pwm_config.instance], G_TIMx_CCR2_CCR2_L, ccr_value);
            break;
        case 3: 
            WRITE_FIELD(G_TIMx_CCR3[pwm_config.instance], G_TIMx_CCR3_CCR3_L, ccr_value);
            break;
        case 4: 
            WRITE_FIELD(G_TIMx_CCR4[pwm_config.instance], G_TIMx_CCR4_CCR4_L, ccr_value);
            break;
        default: 
            *errc = TI_ERRC_INVALID_ARG;
            return;
    }
    // asm("BKPT #0");
    // Set to output compare
    pwm_setup_output_compare(pwm_config.instance, pwm_config.channel);
    // asm("BKPT #0");
    //Enable the PWM channel
    pwm_enable_channel(pwm_config.instance, pwm_config.channel);
    // asm("BKPT #0");
}