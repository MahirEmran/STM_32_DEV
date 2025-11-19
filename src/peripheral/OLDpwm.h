/**
 * This file is part of the Titan Flight Computer Project
 * Copyright (c) 2024 UW SARP
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
 * @file common/platform/pwm.h
 * @authors Ethan Rad
 * @brief Driver for pulse-width modulation
 */

#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  int pin;            // Pin number on MCU
  int32_t *pinreg;    // Pin reg pointer
  int num_in_group;   // Pin # in GPIO group
  int channel;        // Timer channel number
  int32_t *base;      // Base register of timer channel
  int frequency;      // PWM frequency on pin
  uint16_t dutyCycle; // Duty cycle of PWM signal on pin
  bool running;       // Whether pin is outputting a PWM signal
} pwm_pin_t;



//----------
// TODO FACT CHECK THESE PINS.  
// THE VERY FIRST ONE WAS WRONG

// ALSO TODO, INCLUDE AF MODE WITH NUMBERS (maybe pin_info struct or smt idk)
//----------


// TIM1 channels and their corresponding pins
// #define TIM1_CH1_1 70
// #define TIM1_CH1_2 119
// #define TIM1_CH2_1 74
// #define TIM1_CH2_2 120
// #define TIM1_CH3_1 76
// #define TIM1_CH3_2 121
// #define TIM1_CH4_1 77
// #define TIM1_CH4_2 122

// TIM2 channels and their corresponding pins
#define TIM2_CH1_1 37  // PA0
#define TIM2_CH1_2 51  // PA5
#define TIM2_CH1_3 138 // PA15
// #define TIM2_CH2_1 41
// #define TIM2_CH2_2 161
// #define TIM2_CH3_1 42
// #define TIM2_CH3_2 79
// #define TIM2_CH4_1 47
// #define TIM2_CH4_2 80

// TIM3 channels and their corresponding pins
// #define TIM3_CH1_1 52
// #define TIM3_CH1_2 115
// #define TIM3_CH1_3 162
#define TIM3_CH2_1 53  // PA7
#define TIM3_CH2_2 116 // PC7
#define TIM3_CH2_3 163 // PB5
// #define TIM3_CH3_1 56
// #define TIM3_CH3_2 117
// #define TIM3_CH4_1 57
// #define TIM3_CH4_2 118

// TIM4 channels and their corresponding pins
#define TIM4_CH1_1 100 // PD12
#define TIM4_CH1_2 164 // PB6
// #define TIM4_CH2_1 101
// #define TIM4_CH2_2 165
// #define TIM4_CH3_1 104
// #define TIM4_CH3_2 167
// #define TIM4_CH4_1 105
// #define TIM4_CH4_2 168

// TIM5 channels and their corresponding pins
// #define TIM5_CH1_1 40
// #define TIM5_CH1_2 87
#define TIM5_CH2_1 41 // PA1
#define TIM5_CH2_2 88 // PH11
// #define TIM5_CH3_1 42
// #define TIM5_CH3_2 89
// #define TIM5_CH4_1 47
// #define TIM5_CH4_2 131



/**
 * @param pin: The integer value of the pin
 * @param frequency: The PWM frequency of the pin
 * @param dutyCycle: The duty cycle of the pwm signal on a scale of 0 to 65535
 */
void tal_pwm_pin_init(int pin, uint32_t frequency, uint16_t dutyCycle, bool *const err);

/**
 * @param pin: The integer value of the pin
 * @param frequency: The PWM frequency of the pin
 */
void tal_pwm_pin_set_channel_freq(int pin, int frequency, bool *const err);

/**
 * @param pin: The integer value of the pin
 * @param dutyCycle: The duty cycle of the pwm signal on a scale of 0 to 255
 */
void tal_pwm_pin_set_channel_duty_cycle(int pin, uint8_t dutyCycle,
                                        bool *const err);

/**
 * @param pin: The pin to enable PWM on
 */
void tal_pwm_pin_enable(int pin, bool *const err);

/**
 * @param pin: The pin to disable PWM on
 */
void tal_pwm_pin_disable(int pin, bool *const err);

/**
 * @param pin: The pin which is to be checked if it is running
 *
 * @return: true if pwm on the pin is on, false if it is off
 */
bool tal_pwm_is_running(int pin, bool *const err);

/**
 * @param pin: The pin which is to be checked if it is valid
 * @param pin_info: Pointer to struct which should be filled with pin info
 *
 * @return: true if pwm on the pin is valid, false if it is not
 */
bool get_pin_info(int pin, pwm_pin_t *pin_info);
