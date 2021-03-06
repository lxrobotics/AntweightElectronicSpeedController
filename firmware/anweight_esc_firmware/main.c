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
* @brief main file for antweight electronic speed control firmware
* @file main.cpp
*/

#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>
#include "motor_control.h"
#include "input.h"
#include "control.h"
#include "config.h"
#include "status_led.h"
#include "VirtualSerial/VirtualSerial.h"

// configuration structure
volatile s_config_data configuration;
// flag to tell control module to perform calibration of neutral position
volatile bool do_calibration_of_neutral_position = false;

/**
* @brief initializes the whole application
*/
void init_application();

typedef enum {INIT = 0, ACTIVE = 1, CALIBRATION = 2, FAILSAFE = 3, CONFIG = 4, ERROR = 5} E_FIRMWARE_STATE;

int main(void) {
	
	init_application();

	uint8_t firmware_state = INIT;
	
	for(;;) {
	
		switch(firmware_state) {
			case INIT: {
				while(!input_good()) {
					// wait until we have a good signal
					// do the usb task necessary for working the usb
					virtual_serial_task();
					// if we have data available, switch the firmware state to go to config mode
					if(virtual_serial_bytes_available()) {
						firmware_state = CONFIG;
						break;
					}
				}				 
				if(firmware_state == INIT) {
					firmware_state = CALIBRATION;
				}
			}	break;
			case CALIBRATION: {
				// now do the calibration, set the config flag
				do_calibration_of_neutral_position = true;
				while(do_calibration_of_neutral_position) {
					// wait for calibration to be done
				}
				// and switch over to avtive state
				firmware_state = ACTIVE;
			} break;
			case ACTIVE: {
				// the input signals are switch to the output signals depending on the driving mode (tank or v mixer)
				// monitor the signals, if there are no pulses on both channels for 5 periods switch to failsafe mode
				enable_motors();
				// turn on status led to signalize operation
				status_led_turn_on();
				while(input_good()) {
					// do the control stuff here, since its interrupt controlled nothing to do here anymore
				}
				firmware_state = FAILSAFE; // input channels are bad, switch to failsafe
			} break;
			case FAILSAFE: {
				// we are in failsafe, so switch of the output channels
				disable_motors();
				// also turn off status led to signal that we are not in active state anylonger
				status_led_turn_off();
				while(!input_good()) {
					// wait until the signals are back up, if thats the case switch back to active
				}
				firmware_state = ACTIVE;
			} break;
			case CONFIG: {
				// do the config, no disabling of the motors needed, since they are not yet enabled
				bool config_done = false;
				do {
					// do the usb task necessary for working the usb
					virtual_serial_task();
					// read from usb, change the settings according to that and send the requests answers
					if(virtual_serial_bytes_available()) {
						uint8_t data_byte = 0;
						if(virtual_serial_receive_byte(&data_byte)) {
							config_parse_data(data_byte, &config_done);
						}
					}
				} while(!config_done);				
				// then go back to init
				firmware_state = INIT; 
			} break;
			case ERROR: {
				// if we should land hear, whatever the reason, switch all output off
				disable_motors();
				// turn status led off, no valid operation mode
				status_led_turn_off();
				// no way leads out of here
				for(;;) { asm("NOP"); }
			} break;
			default: {
				firmware_state = ERROR;
			} break;
		}
	}
}

/**
* @brief initializes the whole application
*/
void init_application() {

	// load parameters from EEPROM
	init_config();
	
	// initialize motor control
	init_motor_control();
	
	// init the contro module
	init_control();
	
	// initialize the input module and register the callbacks
	init_input(control_ch1_data_callback, control_ch2_data_callback);
	
	// initialize the virtual serial
	init_virtual_serial();
	
	// initialize the status led
	init_status_led();
	
	// enable globally interrupts
	sei();
}