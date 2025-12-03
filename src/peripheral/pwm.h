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
* @file peripheral/pwm.h
* @authors Joshua Beard, Aaron Mcbride, Jude Merritt
* @brief Interface for the PWM driver  
* */

#pragma once

#include <stdint.h>
#include "peripheral/errc.h"


/**************************************************************************************************
* @section Type Definitions
**************************************************************************************************/

struct ti_pwm_config_t {
    int32_t channel;
    int32_t instance;
    int32_t freq;
    int32_t duty;
    int32_t clock_freq;
};


/**************************************************************************************************
* @section Function Definitions
**************************************************************************************************/

/** 
* @brief Configures a timer and GPIO pin to output PWM at the specified frequency and duty cycle.
* 
* @param pwm_config Takes information from the pwm_config structure
* @param errc Pointer to error code
*/
void ti_set_pwm(struct ti_pwm_config_t pwm_config, enum ti_errc_t* errc);