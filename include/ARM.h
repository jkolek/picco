// ARM (Cortex) header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef ARM_H
#define ARM_H

//
// ARM GPR registers
//

enum
{
    ARM_REG_R0,
    ARM_REG_R1,
    ARM_REG_R2,
    ARM_REG_R3,
    ARM_REG_R4,
    ARM_REG_R5,
    ARM_REG_R6,
    ARM_REG_R7,
    ARM_REG_R8,
    ARM_REG_R9,
    ARM_REG_R10,
    ARM_REG_R11,
    ARM_REG_R12,
    ARM_REG_R13, // Stack Pointer (SP)
    ARM_REG_R14, // Link Register (LR)
    ARM_REG_R15, // Program Counter (PC)
};

#define ARM_REG_SP ARM_REG_R13
#define ARM_REG_LR ARM_REG_R14
#define ARM_REG_PC ARM_REG_R15

//
// ARM FPU registers
//

//
// ARM Cortex instruction opcodes
//

// Condition codes
#define CC_EQ 0x0
#define CC_NE 0x1
#define CC_CS 0x2
#define CC_CC 0x3
#define CC_MI 0x4
#define CC_PL 0x5
#define CC_VS 0x6
#define CC_VC 0x7
#define CC_HI 0x8
#define CC_LS 0x9
#define CC_GE 0xa
#define CC_LT 0xb
#define CC_GT 0xc
#define CC_LE 0xd
#define CC_AL 0xe

// Data processing instructions
//
//  |31  28|27  26| 25 |24    21| 20 |19 16|15 12|11        0|
//  +--------------------------------------------------------+
//  | Cond |  00  |  I | OpCode |  S | Rn  | Rd  | Operand 2 |
//  +--------------------------------------------------------+

// OpCodes
#define ARM_AND 0x0
#define ARM_EOR 0x1
#define ARM_SUB 0x2
#define ARM_RSB 0x3
#define ARM_ADD 0x4
#define ARM_ADC 0x5
#define ARM_SBC 0x6
#define ARM_RSC 0x7
#define ARM_TST 0x8
#define ARM_TEQ 0x9
#define ARM_CMP 0xa
#define ARM_CMN 0xb
#define ARM_ORR 0xc
#define ARM_MOV 0xd
#define ARM_BIC 0xe
#define ARM_MVN 0xf

// Branch and Branch with Link instructions (B, BL)
//
//  |31  28|27 25| 24 |23                                   0|
//  +--------------------------------------------------------+
//  | Cond | 101 |  L | Offset                               |
//  +--------------------------------------------------------+

#define ARM_B 0x0
#define ARM_BL 0x0

// Branch and Exchange instruction (BX)

#define ARM_BX 0x0

// Single Data Transfer (LDR, STR)

#define ARM_LDR 0x0
#define ARM_STR 0x0

#define ARM_CDP 0x0
#define ARM_LDC 0x0
#define ARM_LDM 0x0
#define ARM_MCR 0x0
#define ARM_MLA 0x0
#define ARM_MCR 0x0
#define ARM_MRS 0x0
#define ARM_MSR 0x0
#define ARM_MUL 0x0
#define ARM_STC 0x0
#define ARM_STM 0x0
#define ARM_SWI 0x0
#define ARM_SWP 0x0

//
//  Static ARM relocation types
//

enum
{
    R_ARM_NONE = 0,
    R_ARM_LDR_PC_G0 = 4,
    R_ARM_ABS12 = 6,
    R_ARM_CALL = 28,
    R_ARM_JUMP24 = 29,
    R_ARM_MOVW_ABS_NC = 43,
    R_ARM_MOVT_ABS = 44,
    R_ARM_MOVW_PREL_NC = 45,
    R_ARM_MOVT_PREL = 46,
    R_ARM_ALU_PC_G0_NC = 57,
    R_ARM_ALU_PC_G0 = 58,
    R_ARM_ALU_PC_G1_NC = 59,
    R_ARM_ALU_PC_G1 = 60,
    R_ARM_ALU_PC_G2 = 61,
    R_ARM_LDR_PC_G1 = 62,
    R_ARM_LDR_PC_G2 = 63,
    R_ARM_LDRS_PC_G0 = 64,
    R_ARM_LDRS_PC_G1 = 65,
    R_ARM_LDRS_PC_G2 = 66,
    R_ARM_LDC_PC_G0 = 67,
    R_ARM_LDC_PC_G1 = 68,
    R_ARM_LDC_PC_G2 = 69,
    R_ARM_ALU_SB_G0_NC = 70,
    R_ARM_ALU_SB_G0 = 71,
    R_ARM_ALU_SB_G1_NC = 72,
    R_ARM_ALU_SB_G1 = 73,
    R_ARM_ALU_SB_G2 = 74,
    R_ARM_LDR_SB_G0 = 75,
    R_ARM_LDR_SB_G1 = 76,
    R_ARM_LDR_SB_G2 = 77,
    R_ARM_LDRS_SB_G0 = 78,
    R_ARM_LDRS_SB_G1 = 79,
    R_ARM_LDRS_SB_G2 = 80,
    R_ARM_LDC_SB_G0 = 81,
    R_ARM_LDC_SB_G1 = 82,
    R_ARM_LDC_SB_G2 = 83,
    R_ARM_MOVW_BREL_NC = 84,
    R_ARM_MOVT_BREL = 85,
    R_ARM_MOVW_BREL = 86,
    R_ARM_GOT_BREL12 = 97,
    R_ARM_GOTOFF12 = 98,
    R_ARM_TLS_LDO12 = 109,
    R_ARM_TLS_LE12 = 110,
    R_ARM_TLS_IE12GP = 111
};

#endif
