/*
	Copyright 2013 by Alexander Entinger, BSc

    This file is part of antweight_esc_firmware.

    antweight_esc_firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    antweight_esc_firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with antweight_esc_firmware.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
* @author Alexander Entinger, BSc
* @brief this file implements the reading from the receiver channels
* @file input.h
*/

#ifndef INPUT_H_
#define INPUT_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {RISING = 0, FALLING = 1} E_EDGE_STATE;
typedef void (*callback_func)(uint16_t);

/**
 * @brief initializes the input module
 */
void init_input(callback_func cb_ch1, callback_func cb_ch2);
	
/** 
 * @brief returns if there were valid pulses in valid periods received
 */
bool input_good();	

#endif /* INPUT_H_ */