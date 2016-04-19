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

#include <stdbool.h>
#include <stdint.h>

/**
 * @breif Setup the FPB to redirect a function call (i.e. call new_func(...) instead of old_func(...)).
 *
 * WARNING: This function will use two FPB.COMP registers (reg_index and reg_index + 1) if the instruction to be replaced is half-word-aligned.
 * @param[in] uint32_t The base address of the patch table.
 * @param[in] uint32_t The address in Code memory where the instruction to be replaced is stored.
 * @param[in] uint32_t The target address to branch to (limited to branches of only +16 MB).
 * @param[in] bool     If true, this function should be used to redirect a specific call to a function. ****IMPORTANT****
                       If false, this function should be used to redirect ALL calls to a specific function.
 * @param[in] uint8_t  The FPB.COMP register(s) to be used (see above WARNING).
 * @return    int    0 if function succeeded, other if a failure occured.
 */
int fpb_redirect_function_call(uint32_t remap_table_addr,
	                             uint32_t instr_addr,
	                             uint32_t target_addr,
                               uint8_t  reg_index,
							                 bool     bl_instr);

/**
 * @brief Function for enabling the flash patch block.
 *
 * Sets the ENABLE bit field in the Flash Patch Control Register to 1 (flash patch unit enabled).
 */
void fpb_control_enable(void);

/**
 * @brief Function for providing the location of the patch table in RAM (System space) where a matched address is remapped.
 *
 * @param[in] uint32_t The REMAP address must be located in RAM and 8-word aligned, with one word allocated to each of the eight FPB comparators.
 */
void fpb_remap_reg_config(uint32_t remap_table_addr);

/**
 * @brief Function for storing the PC addresses in the Flash Patch Comparator Registers to be compared against.
 *
 * On a match, the instruction stored at comp_addr will be replaced with the instruction stored at PATCH_TABLE[reg_index].
 * @param[in] uint8_t  reg_index The index of the FP Comparator Register to configure.
 * @param[in] uint32_t comp_addr The PC address to compare against.
 */
void fpb_comparator_reg_config(uint8_t reg_index, uint32_t comp_addr);

/**
 * @brief Function for calculating the bit pattern for the new Branch (B encoding T4) instruction.
 *
 * Note: See https://web.eecs.umich.edu/~prabal/teaching/eecs373-f10/readings/ARMv7-M_ARM.pdf for info on instructions.
 * @param[in] uint32_t instr_addr  The address in Code memory where the instruction to be replaced is stored.
 * @param[in] uint32_t target_addr The target address to branch to (limited to branches of only +16 MB).
 * @return    uint32_t             The new 32-bit Branch instruction.
 */
uint32_t calc_branch_instr(uint32_t instr_addr, uint32_t target_addr);

/**
 * @brief Function for calculating the bit pattern for the new Branch with Link (BL) instruction.
 *
 * @param[in] uint32_t instr_addr  The address in Code memory where the instruction to be replaced is stored.
 * @param[in] uint32_t target_addr The target address to branch to (limited to branches of only +16 MB).
 * @return    uint32_t             The new 32-bit Branch with Link instruction.
 */
uint32_t calc_branch_w_link_instr(uint32_t instr_addr, uint32_t target_addr);

/**
 * @brief Used for formatting 32 bit instructions.
 *
 * @param[in] uint32_t val UPPER_16_BITS, LOWER_16_BITS
 * @return    uint32_t The 16 bit little endian representation. LOWER_16_BITS, UPPER_16_BITS
 */
uint32_t little_endian_16_bit(uint32_t val);

/**
 * Test.
 */
int test_calc_remap_bl_instr(uint32_t old_bl_instr_addr, uint32_t new_bl_target_addr, uint32_t new_bl_instr);

/**
 * Sanity test to help catch problems with the debugger interfering with the FPB.
 */
int test_fpb_configured_properly(uint32_t remap_table_addr, uint8_t reg_index, uint32_t comp_addr, uint32_t new_bl_instr);
