// ARM decoder - implementation file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#include <cassert>
#include <cstdio>

#include "../include/ARM.h"
#include "../include/ARMDecoder.h"

const char *ARMDecoder::opToStr(unsigned op)
{
    switch (op)
    {
        default:
            assert(0 && "unknown opcode");
    }
}

const char *ARMDecoder::opToStrSpecial(unsigned op)
{
    switch (op)
    {
        default:
            assert(0 && "unknown opcode");
    }
}

const char *ARMDecoder::opToStrSpecial2(unsigned op)
{
    switch (op)
    {
        default:
            assert(0 && "unknown opcode");
    }
}

const char *ARMDecoder::opToStrCOP1(unsigned op)
{
    switch (op)
    {
        default:
            assert(0 && "unknown opcode");
    }
}

const char *ARMDecoder::regToStr(unsigned reg)
{
    switch (reg)
    {
        case ARM_REG_R0:
            return "r0";
        case ARM_REG_R1:
            return "r1";
        case ARM_REG_R2:
            return "r2";
        case ARM_REG_R3:
            return "r3";
        case ARM_REG_R4:
            return "r4";
        case ARM_REG_R5:
            return "r5";
        case ARM_REG_R6:
            return "r6";
        case ARM_REG_R7:
            return "r7";
        case ARM_REG_R8:
            return "r8";
        case ARM_REG_R9:
            return "r9";
        case ARM_REG_R10:
            return "r10";
        case ARM_REG_R11:
            return "r11";
        case ARM_REG_R12:
            return "r12";
        case ARM_REG_R13:
            return "sp";
        case ARM_REG_R14:
            return "lr";
        case ARM_REG_R15:
            return "pc";
        default:
            assert(0 && "Unknown register");
    }
}

const char *regListToStr(unsigned regList) { return "regList"; }

const char *ARMDecoder::regFPToStr(unsigned reg)
{
    switch (reg)
    {
        default:
            assert(0 && "unknown FPU register");
    }
}

const char *ccToStr(unsigned cc)
{
    switch (cc)
    {
        case CC_EQ:
            return "eq";
        case CC_NE:
            return "ne";
        case CC_CS:
            return "cs";
        case CC_CC:
            return "cc";
        case CC_MI:
            return "mi";
        case CC_PL:
            return "pl";
        case CC_VS:
            return "vs";
        case CC_VC:
            return "vc";
        case CC_HI:
            return "hi";
        case CC_LS:
            return "ls";
        case CC_GE:
            return "ge";
        case CC_LT:
            return "lt";
        case CC_GT:
            return "gt";
        case CC_LE:
            return "le";
        case CC_AL:
            return "";
        default:
            assert(0 && "unknown condition code");
    }
}

static unsigned getRn(unsigned binary) { return (binary >> 16) % 0x10; }

static unsigned getRd(unsigned binary) { return (binary >> 12) % 0x10; }

static unsigned getRs(unsigned binary) { return (binary >> 8) % 0x10; }

static unsigned getRm(unsigned binary) { return binary % 0x10; }

//static unsigned getOperand2(unsigned binary)
//{
//    unsigned imm = (binary >> 25) % 0x2;
//    if (imm)
//    return (binary )
//}

static const char *shiftTypeToStr(unsigned shiftType)
{
    switch (shiftType)
    {
        case 0:
            return "lsl";
        case 1:
            return "lsr";
        case 2:
            return "asl";
        case 3:
            return "asr";
        default:
            assert(0 && "Unknown shift type");
    }
}

void ARMDecoder::decode(unsigned binary)
{
    unsigned rd, rn, rm, rt, rs, sa, uimm8, base, offset, code, shift, rotate;
    unsigned decbits, opcode, operand2, operand2IsImm, instClass, instClass1,
        cc;

    cc = (binary >> 28);

    // Get decode bits
    decbits = (binary >> 20) % 0x100;
    instClass = (decbits >> 6);
    instClass1 = (decbits >> 5);
    if (decbits == 0x12)
    {
        rn = binary % 0x10;
        printf("bx%s\t%s\n", ccToStr(cc), regToStr(rn));
    }
    else if ((decbits >> 2) == 0x0)
    {
        unsigned accumulate;

        // Multiply and Multiply-Accumulate (MUL, MLA)
        rn = getRn(binary);
        rd = getRd(binary);
        rs = getRs(binary);
        rm = getRm(binary);
        accumulate = (decbits >> 1) % 0x2;
        if (accumulate)
            printf("mla%s\t%s, %s, %s, %s\n", ccToStr(cc), regToStr(rd),
                   regToStr(rm), regToStr(rs), regToStr(rn));
        else
            printf("mul%s\t%s, %s, %s\n", ccToStr(cc), regToStr(rd),
                   regToStr(rm), regToStr(rs));
    }
    else if (instClass1 == 0x4)
    {
        rn = getRn(binary);
        if ((decbits & 0x1) == 0x1)
        {
            // Load block from memory instruction
            printf("ldm%s\t%s, {0x%x}\n", ccToStr(cc), regToStr(rn),
                   (binary % 0x10000));
        }
        else
        {
            // Store block to memory instruction
            printf("stm%s\t%s, {0x%x}\n", ccToStr(cc), regToStr(rn),
                   (binary % 0x10000));
        }
        // printf("stm%s\t%s,%x\n", ccToStr(cc), regToStr(rn), (binary %
        // 0x10000));
    }
    else if (instClass1 == 0x5)
    {
        // FIXME: This cannot be short, it must be int!
        short offset;
        unsigned linkBit;

        offset = (binary % 0x1000000);
        linkBit = ((binary >> 24) % 0x2);

        if (linkBit)
            printf("bl%s\t%d\n", ccToStr(cc), (offset << 2));
        else
            printf("b%s\t%d\n", ccToStr(cc), (offset << 2));
    }
    else if (instClass == 0x0)
    {
        unsigned shiftType, isRsReg, shiftAmount;

        opcode = (decbits >> 1) % 0x10;
        rn = getRn(binary);
        rd = getRd(binary);
        operand2IsImm = (binary >> 25) % 0x2;
        operand2 = binary % 0x1000;
        shiftType = 0;
        isRsReg = 0;
        shiftAmount = 0;

        if (operand2IsImm)
        {
            rotate = operand2 >> 8;
            uimm8 = operand2 % 0x100;
        }
        else
        {
            //
            //  11           7 6  5  4
            //  +-------------+----+---+
            //  |             |    | 0 |
            //  +-------------+----+---+
            //
            //  Shift type (bits 6-5):
            //  00 = logical left
            //  01 = logical right
            //  10 = arithmetic right
            //  11 = rotate right
            //
            //  Shift amount (bits 11-7):
            //  5 bit unsigned integer
            //
            //  11       8  7  6  5  4
            //  +-------------+----+---+
            //  |   Rs    | 0 |    | 1 |
            //  +-------------+----+---+
            //
            //  Shift type (bits 6-5):
            //  00 = logical left
            //  01 = logical right
            //  10 = arithmetic right
            //  11 = rotate right
            //
            //  Shift amount (bits 11-8):
            //  Shift amount specified in bottom byte of Rs

            rm = operand2 % 0x10;
            shift = operand2 >> 4;
            shiftType = (shift >> 1) % 0x4;
            isRsReg = shift % 0x2;
            shiftAmount = 0;
            if (isRsReg)
            {
                rs = shift >> 4;
            }
            else
            {
                shiftAmount = shift >> 3;
            }
        }
        switch (opcode)
        {
            case ARM_AND:
                if (operand2IsImm)
                {
                    printf("and%s\t%s, %s, #%d\n", ccToStr(cc), regToStr(rd),
                           regToStr(rn), uimm8);
                }
                else
                {
                    printf("and%s\t%s, %s, %s\n", ccToStr(cc), regToStr(rd),
                           regToStr(rn), regToStr(rm));
                }
                break;
            case ARM_EOR:
                if (operand2IsImm)
                {
                    printf("eor%s\t%s, %s, #%d\n", ccToStr(cc), regToStr(rd),
                           regToStr(rn), operand2);
                }
                else
                {
                    printf("eor%s\t%s, %s, %s\n", ccToStr(cc), regToStr(rd),
                           regToStr(rn), regToStr(operand2));
                }
                break;
            case ARM_SUB:
                if (operand2IsImm)
                {
                    printf("sub%s\t%s, %s, #%d\n", ccToStr(cc), regToStr(rd),
                           regToStr(rn), operand2);
                }
                else
                {
                    printf("sub%s\t%s, %s, %s\n", ccToStr(cc), regToStr(rd),
                           regToStr(rn), regToStr(operand2));
                }
                break;
            case ARM_RSB:
                if (operand2IsImm)
                {
                    printf("rsb%s\t%s, %s, #%d\n", ccToStr(cc), regToStr(rd),
                           regToStr(rn), operand2);
                }
                else
                {
                    printf("rsb%s\t%s, %s, %s\n", ccToStr(cc), regToStr(rd),
                           regToStr(rn), regToStr(operand2));
                }
                break;
            case ARM_ADD:
                if (operand2IsImm)
                {
                    printf("add%s\t%s, %s, #%d\n", ccToStr(cc), regToStr(rd),
                           regToStr(rn), operand2);
                }
                else
                {
                    printf("add%s\t%s, %s, %s\n", ccToStr(cc), regToStr(rd),
                           regToStr(rn), regToStr(operand2));
                }
                break;
            case ARM_ADC:
                // if (operand2IsImm)
                //  R[rd] = R[Rn] + operand2 + C;
                // else
                //  R[rd] = R[Rn] + R[operand2] + C;
                break;
            case ARM_SBC:
                // if (operand2IsImm)
                //  R[rd] = R[Rn] - operand2 + C - 1;
                // else
                //  R[rd] = R[Rn] - R[operand2] + C - 1;
                break;
            case ARM_RSC:
                break;
            case ARM_TST:
                break;
            case ARM_TEQ:
                break;
            case ARM_CMP:
                printf(
                    (operand2IsImm ? "cmp%s\t%s, #%d\n" : "cmp%s\t%s, r%d\n"),
                    ccToStr(cc), regToStr(rn), operand2);
                break;
            case ARM_CMN:
                break;
            case ARM_ORR:
                printf((operand2IsImm ? "orr%s\t%s, %s, #%d\n"
                                      : "orr%s\t%s, %s, r%d\n"),
                       ccToStr(cc), regToStr(rd), regToStr(rn), operand2);
                break;
            case ARM_MOV:
                if (operand2IsImm)
                {
                    printf("mov%s\t%s, #%d\n", ccToStr(cc), regToStr(rd),
                           uimm8);
                }
                else
                {
                    if (!isRsReg && shiftAmount)
                    {
                        printf("mov%s\t%s, %s %s\n", ccToStr(cc), regToStr(rd),
                               shiftTypeToStr(shiftType), regToStr(rm));
                    }
                    else
                    {
                        printf("mov%s\t%s, %s\n", ccToStr(cc), regToStr(rd),
                               regToStr(rm));
                    }
                }
                break;
            case ARM_BIC:
                break;
            case ARM_MVN:
                printf(
                    (operand2IsImm ? "mvn%s\t%s, #%d\n" : "mvn%s\t%s, r%d\n"),
                    ccToStr(cc), regToStr(rd), operand2);
                // printf("mov%s\t%s, #%d\n", ccToStr(cc), regToStr(rd),
                // operand2);
                break;
            default:
                break;
        }
    }
    else if (instClass == 0x1)
    {
        unsigned transfB;

        rn = getRn(binary);
        rd = getRd(binary);
        // operand2IsImm = (binary >> 25) % 0x2;
        offset = binary % 0x1000;
        transfB = (0x1 & (decbits >> 2));
        if ((decbits & 0x1) == 0x1)
        {
            // Load from memory instruction
            if (transfB)
                printf("ldr%sb\t%s, [%s, #%d]\n", ccToStr(cc), regToStr(rd),
                       regToStr(rn), offset);
            else
                printf("ldr%s\t%s, [%s, #%d]\n", ccToStr(cc), regToStr(rd),
                       regToStr(rn), offset);
        }
        else
        {
            // Store to memory instruction
            if (transfB)
                printf("str%sb\t%s, [%s, #%d]\n", ccToStr(cc), regToStr(rd),
                       regToStr(rn), offset);
            else
                printf("str%s\t%s, [%s, #%d]\n", ccToStr(cc), regToStr(rd),
                       regToStr(rn), offset);
        }
    }
}

char ARMDecoder::getHexStr(unsigned int n)
{
    if (n >= 0x0 && n <= 0x9)
        return n + 48;
    return n + 87;
}

void ARMDecoder::printHexa(unsigned binary)
{
    for (int i = 28; i >= 0; i -= 4)
        printf("%c", getHexStr((binary >> i) & 0xf));
}

const char *ARMDecoder::getRelocName(unsigned reloc)
{
    switch (reloc)
    {
        case R_ARM_CALL:
            return "R_ARM_CALL";
        default:
            assert(0 && "Unknown relocation type");
    }
}

void ARMDecoder::printBuffer()
{
    unsigned binary;

    printf("Code buffer:\n\n");
    for (unsigned i = 0; i < PC; i += 4)
    {
        binary = get4b(i);
        printf(" %d:\t", i);
        printHexa(binary);
        printf("\t");
        decode(binary);
        for (unsigned n = 0; n < relocsc; n++)
        {
            if (relocs[n].r_offset == i)
            {
#ifndef GNU_ABI
                printf("\t\t\t%d: %s  %s\n", relocs[n].r_offset,
                       getRelocName(ELF32_R_TYPE(relocs[n].r_info)),
                       relocs[n].value);
#endif
            }
        }
    }
    printf("\n");
}
