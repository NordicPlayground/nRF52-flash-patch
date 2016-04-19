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

#include <nrf.h>

#include "flash_patch.h"
#include "fpb_lib.h"

#define REMAP_TABLE_ADDR (0x20007000UL)

#define INSTR_ADDR       (0x0000097CUL)
#define TARGET_ADDR      (0x0007F030UL)
#define REG_INDEX        (0x04UL)

int fpb_setup()
{
	int error_code = 0;
	
	#ifdef DEBUG
		return error_code;
	#endif // DEBUG
	
	fpb_control_enable();
	fpb_remap_reg_config(REMAP_TABLE_ADDR);
	// We are replacing the function with it's first instruction at INSTR_ADDR (turn_led_off) with the function with it's first instruction at TARGET_ADDR (turn_led_on).
	error_code = fpb_redirect_function_call(REMAP_TABLE_ADDR, INSTR_ADDR, TARGET_ADDR, REG_INDEX, false);
	
	return error_code;
}

// This is the patched function.
void turn_led_on(uint32_t pin)
{	
	NRF_GPIO->OUTCLR = (1UL << pin);
}
