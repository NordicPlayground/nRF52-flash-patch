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

#include "fpb_lib.h"

typedef struct
{
    __IO uint32_t CTRL;
    __IO uint32_t REMAP;
    __IO uint32_t COMP[8];
} FPB_t;

#define FPB_BASE        (0xE0002000UL)
#define FPB             ((FPB_t *) FPB_BASE)

int fpb_redirect_function_call(uint32_t remap_table_addr,
	                             uint32_t instr_addr,
	                             uint32_t target_addr,
                               uint8_t  reg_index,
							                 bool     bl_instr)
{
	uint32_t old_instr[2]; // Only used if instruction to replace is half-word aligned.
	uint32_t new_instr;
	
	if (bl_instr)
	{
		new_instr = calc_branch_w_link_instr(instr_addr, target_addr);
	}
	else // Branch instruction.
	{
		new_instr = calc_branch_instr(instr_addr, target_addr);
	}
	
	if (instr_addr % 4 == 0) // Instruction is word aligned.
	{
		fpb_comparator_reg_config(reg_index, instr_addr);
		*((uint32_t *) (remap_table_addr + (reg_index * 4))) = little_endian_16_bit(new_instr);
	}
	else // Instruction is half-word aligned.
	{
		old_instr[0] = *((uint32_t *)  (instr_addr & 0xFFFFFFFC));
		old_instr[1] = *((uint32_t *) ((instr_addr & 0xFFFFFFFC) + 4));
		
		fpb_comparator_reg_config(reg_index, instr_addr & 0xFFFFFFFC);
		fpb_comparator_reg_config(reg_index + 1, (instr_addr & 0xFFFFFFFC) + 4);
		
		*((uint32_t *) (remap_table_addr + reg_index * 4))       = ((little_endian_16_bit(new_instr) & 0x0000FFFF) << 16) | (old_instr[0] & 0x0000FFFF);
		*((uint32_t *) (remap_table_addr + (reg_index + 1) * 4)) = (old_instr[1] & 0xFFFF0000) | ((little_endian_16_bit(new_instr) & 0xFFFF0000) >> 16);
	}
	
	return 0;
}

void fpb_control_enable(void)
{
	FPB->CTRL = 0x03UL;
}

void fpb_remap_reg_config(uint32_t remap_table_addr)
{
	FPB->REMAP = remap_table_addr;
}

void fpb_comparator_reg_config(uint8_t reg_index, uint32_t comp_addr)
{
	FPB->COMP[reg_index] = (comp_addr | 0x01UL);
}

uint32_t calc_branch_instr(uint32_t instr_addr, uint32_t target_addr)
{
	uint32_t offset = target_addr - instr_addr;
	uint16_t offset_10_upper = (offset >> 12) & 0x03FF;
	uint16_t offset_11_lower = ((offset - 4) >> 1)  & 0x07FF; // UNCERTAIN about this!
	
	uint8_t s_pos = 24;
	uint8_t s  = (offset - 4) & (1 << s_pos);
	uint8_t i1 = (offset - 4) & (1 << (s_pos - 1));
	uint8_t i2 = (offset - 4) & (1 << (s_pos - 2));
	
	uint8_t j1 = 0x01 & ((~i1) ^ s);
	uint8_t j2 = 0x01 & ((~i2) ^ s);
	
	uint16_t upper_bl_instr =  ((0x1E << 11) | (s << 10) | offset_10_upper);
	uint16_t lower_bl_instr =  ((0x02 << 14) | (j1 << 13) | (0x01 << 12) | (j2 << 11) | offset_11_lower);
	
	return ((upper_bl_instr << 16) | lower_bl_instr);
}

uint32_t calc_branch_w_link_instr(uint32_t instr_addr, uint32_t target_addr)
{
	uint32_t branch_instr = calc_branch_instr(instr_addr, target_addr);
	return (branch_instr | 0x00004000); // Set bit 14. This is the only difference between B and BL instructions.
}


/********** HELPERS **********/


uint32_t little_endian_16_bit(uint32_t val)
{
	return ((val & 0xFFFF0000) >> 16) | ((val & 0x0000FFFF) << 16);
}


/********** TESTS **********/


int test_calc_remap_bl_instr(uint32_t old_bl_instr_addr, uint32_t new_bl_target_addr, uint32_t new_bl_instr)
{
	uint8_t s  = (new_bl_instr >> 26) & 0x01;
	uint8_t j1 = (new_bl_instr >> 13) & 0x01;
	uint8_t j2 = (new_bl_instr >> 11) & 0x01;
	
	uint8_t i1 = (~(j1 ^ s)) & 0x01;
	uint8_t i2 = (~(j2 ^ s)) & 0x01;
	
	uint16_t offset_10_upper = (new_bl_instr >> 16) & 0x03FF;
	uint16_t offset_11_lower = new_bl_instr & 0x07FF;
	
	uint8_t s_pos = 24;
	uint32_t value = ((s << s_pos) | (i1 << (s_pos - 1)) | (i2 << (s_pos - 2)) | (offset_10_upper << 12) | (offset_11_lower << 1));
	
	uint32_t test = new_bl_target_addr - old_bl_instr_addr - 4;
	
	if (test != value)
	{
		return -1;
	}

	return 0;
}

int test_fpb_configured_properly(uint32_t remap_table_addr, uint8_t reg_index, uint32_t comp_addr, uint32_t new_bl_instr)
{
	if(remap_table_addr % 32 != 0)
	{
		return -1;
	}
	
	if ((FPB->CTRL & 0x01) != 0x01)
	{
		return -1;
	}
	
	if (FPB->REMAP != remap_table_addr)
	{
		return -1;
	}
	
	if (FPB->COMP[reg_index] != (comp_addr | 0x01UL))
	{
		return -1;
	}
	
	if (*((uint32_t *) (remap_table_addr + reg_index * 4)) != new_bl_instr)
	{
		return -1;
	}
	
	return 0;
}
