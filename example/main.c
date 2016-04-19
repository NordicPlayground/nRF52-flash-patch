/* Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include <stdint.h>
#include <nrf.h>

#include "flash_patch.h"

#define LED1             (17UL)
#define LED2             (18UL)
#define LED3             (19UL)
#define LED4             (20UL)

static void config_led(uint32_t pin_number)
{
	// Configure GPIO pin as output with standard drive strength.
  NRF_GPIO->PIN_CNF[pin_number] = (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos) |
                                  (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) |
                                  (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
                                  (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos) |
                                  (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
}

static void turn_led_off(uint32_t pin)
{
	NRF_GPIO->OUTSET = (1UL << pin);  
}

int main(void)
{
	fpb_setup();
	
	config_led(LED1);
	turn_led_off(LED1); // We want to redirect this function call with the FPB unit, and instead call turn_led_on(LED1), located at 0x7F000, defined in 'new_function.c.'
	
	config_led(LED2);
	turn_led_off(LED2);
	
	config_led(LED3);
	turn_led_off(LED3);

	for( ; ; );
}

void HardFault_Handler(void)
{
	config_led(LED2);
	config_led(LED3);
	config_led(LED4);
	
	for( ; ; );
}
