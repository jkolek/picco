// ARMv7 (Cortex) Emulator, v0.1
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

// Memory organization
//
//
//  0x7fffffff  +==============+
//              |              |
//              |    Stack     |
//              |              |
//       SP ==> +--------------+
//              |      ||      |
//              |      \/      |
//              |              |
//              |              |
//              |              |
//              |      /\      |
//              |      ||      |
//              +--------------+
//              |              |
//              |     Heap     |
//              |              |
//              +--------------+
//              |              |
//              |  Static data |
//              |              |
//       GP ==> +--------------+
//              |              |
//              |     Code     |
//              |              |
//  0x00000000  +==============+

#include "../include/ARM.h"
#include "../include/PiccoObjectFormat.h"

#include <stdio.h>
#include <stdlib.h>

#define MEMSIZE 65876
//#define MEMSIZE         32768
#define REG_NUM 16
#define TRUE 1
#define FALSE 0

unsigned char M[MEMSIZE];
unsigned R[REG_NUM];
unsigned HI, LO;
unsigned target;
int debug = FALSE;

struct objheader_t exeheader;

// Flags
unsigned C, N, Z, V;

static void setCPSR(unsigned val)
{
    Z = (val == 0) ? TRUE : FALSE;
    // FIXME: Is this right?
    N = ((val & 0x80000000) >> 31);
}

static unsigned getRn(unsigned binary) { return (binary >> 16) % 0x10; }

static unsigned getRd(unsigned binary) { return (binary >> 12) % 0x10; }

static unsigned getRs(unsigned binary) { return (binary >> 8) % 0x10; }

static unsigned getRm(unsigned binary) { return binary % 0x10; }

/*static unsigned getOperand2(unsigned binary)
{
    unsigned imm = (binary >> 25) % 0x2;
    // if (imm)
    // return (binary )
}*/

static void storeWordToMem(unsigned offset, unsigned data)
{
    M[offset] = (unsigned char)data;
    M[offset + 1] = (unsigned char)data >> 8;
    M[offset + 2] = (unsigned char)data >> 16;
    M[offset + 3] = (unsigned char)data >> 24;
}

static unsigned loadWordFromMem(unsigned offset)
{
    unsigned data;

    data = M[offset];
    data |= (M[offset + 1] << 8);
    data |= (M[offset + 2] << 16);
    data |= (M[offset + 3] << 24);
    return data;
}

void executeBranchInst(unsigned binary)
{
    // FIXME: This cannot be short, it must be int!
    short offset;
    unsigned linkBit;

    // B and BL
    offset = (binary % 0x1000000) << 2;
    linkBit = ((binary >> 24) % 0x2);
    if (linkBit)
    {
        // At this point PC is already pointing to a next instruction.
        R[ARM_REG_LR] = R[ARM_REG_PC];
    }
    R[ARM_REG_PC] += (offset - 4);
}

void executeDataProcessingInst(unsigned binary)
{
    unsigned rd, rn, rm, rs, uimm8, shift, rotate, opcode, operand2,
        operand2IsImm;
    unsigned shiftType, isRsReg, shiftAmount, S, decbits;

    decbits = (binary >> 20) % 0x100;
    opcode = (decbits >> 1) % 0x10;
    rn = getRn(binary);
    rd = getRd(binary);
    operand2IsImm = (binary >> 25) % 0x2;
    S = (binary >> 20) % 0x2;
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
        //  11           7 6  5  4
        //  +-------------+----+---+
        //  |             |    | 0 |
        //  +-------------+----+---+
        //
        //  Shift type (bits 6-5):
        //    00 = logical left
        //    01 = logical rigth
        //    10 = arithmetic right
        //    11 = rotate right
        //
        //  Shift amount (bits 11-7):
        //    5 bit unsigned integer
        //
        //  11       8  7  6  5  4
        //  +-------------+----+---+
        //  |   Rs    | 0 |    | 1 |
        //  +-------------+----+---+
        //
        //  Shift type (bits 6-5):
        //    00 = logical left
        //    01 = logical rigth
        //    10 = arithmetic right
        //    11 = rotate right
        //
        //  Shift amount (bits 11-8):
        //    Shift amount specified in bottom byte of Rs

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

    if (!operand2IsImm && !isRsReg && shiftAmount)
    {
        switch (shiftType)
        {
            case 0:
                R[rm] = R[rm] << shiftAmount;
                break; // lsl
            case 1:
                R[rm] = R[rm] >> shiftAmount;
                break; // lsr
            case 2:
                R[rm] = (unsigned)(((int)R[rm]) << shiftAmount);
                break; // asl
            case 3:
                R[rm] = (unsigned)(((int)R[rm]) >> shiftAmount);
                break; // asr
            default:
                printf("Unknown shift type");
        }
    }

    switch (opcode)
    {
        case ARM_AND:
            if (operand2IsImm)
                R[rd] = R[rn] & uimm8;
            else
                R[rd] = R[rn] & R[rm];
            break;
        case ARM_EOR:
            if (operand2IsImm)
            {
                // printf("eor%s\t%s, %s, #%d\n",
                //       ccToStr(cc), regToStr(rd), regToStr(rn), operand2);
            }
            else
            {
                // printf("eor%s\t%s, %s, %s\n",
                //       ccToStr(cc), regToStr(rd), regToStr(rn),
                //       regToStr(operand2));
            }
            break;
        case ARM_SUB:
            if (operand2IsImm)
                R[rd] = R[rn] - operand2;
            else
                R[rd] = R[rn] - R[operand2];
            if (S)
                setCPSR(R[rd]);
            break;
        case ARM_RSB:
            if (operand2IsImm)
                R[rd] = operand2 - R[rn];
            else
                R[rd] = R[operand2] - R[rn];
            break;
        case ARM_ADD:
            if (operand2IsImm)
                R[rd] = R[rn] + operand2;
            else
                R[rd] = R[rn] + R[operand2];
            break;
        case ARM_ADC:
            if (operand2IsImm)
                R[rd] = R[rn] + operand2 + C;
            else
                R[rd] = R[rn] + R[operand2] + C;
            break;
        case ARM_SBC:
            if (operand2IsImm)
                R[rd] = R[rn] - operand2 + C - 1;
            else
                R[rd] = R[rn] - R[operand2] + C - 1;
            break;
        case ARM_RSC:
            break;
        case ARM_TST:
            break;
        case ARM_TEQ:
            break;
        case ARM_CMP:
        {
            int x;

            if (operand2IsImm)
                x = (int)R[rn] - (int)operand2;
            else
                x = (int)R[rn] - (int)R[operand2];

            setCPSR(x);
        }
        break;
        case ARM_CMN:
        {
            int x;

            if (operand2IsImm)
                x = (int)R[rn] + (int)operand2;
            else
                x = (int)R[rn] + (int)R[operand2];

            setCPSR(x);
        }
        break;
        case ARM_ORR:
            // printf((operand2IsImm ? "orr%s\t%s, %s, #%d\n"
            //                      : "orr%s\t%s, %s, r%d\n"),
            //       ccToStr(cc), regToStr(rd), regToStr(rn), operand2);
            break;
        case ARM_MOV:
            if (operand2IsImm)
            {
                R[rd] = uimm8;
            }
            else
            {
                if (!isRsReg && shiftAmount)
                {
                    // printf("mov%s\t%s, %s %s\n",
                    //       ccToStr(cc), regToStr(rd),
                    //       shiftTypeToStr(shiftType), regToStr(rm));
                }
                else
                {
                    // printf("mov%s\t%s, %s\n",
                    //       ccToStr(cc), regToStr(rd), regToStr(rm));
                    R[rd] = R[rm];
                }
            }
            break;
        case ARM_BIC:
            if (operand2IsImm)
                R[rd] = R[rn] & ~uimm8;
            else
                R[rd] = R[rn] & ~R[rm];
            break;
        case ARM_MVN:
            if (operand2IsImm)
                R[rd] = ~uimm8;
            else
                R[rd] = ~R[rm];
            break;
        default:
            printf("Unknown instruction opcode: 0x%x\n", opcode);
            break;
    }
}

void execute(unsigned binary)
{
    unsigned rd, rn, rm, rt, rs, sa, uimm8, base, offset, code, shift, rotate;
    unsigned decbits, opcode, operand2, operand2IsImm, instClass, instClass1,
        cc;

    // printf("PC = %d :: binary = 0x%x\n", R[ARM_REG_PC], binary);

    cc = (binary >> 28);

    // Check condition field
    if (!((cc == CC_EQ && Z) || (cc == CC_NE && !Z) || (cc == CC_CS && C) ||
          (cc == CC_CC && !C) || (cc == CC_MI && N) || (cc == CC_PL && !N) ||
          (cc == CC_VS && V) || (cc == CC_VC && !N) ||
          (cc == CC_HI && (C && !Z)) || (cc == CC_LS && (!C && Z)) ||
          (cc == CC_GE && (N == V)) || (cc == CC_LT && (N != V)) ||
          (cc == CC_GT && (!Z && (N == V))) ||
          (cc == CC_LE && (Z || (N != V))) || (cc == CC_AL)))
    {
        return;
    }

    // Get decode bits
    decbits = (binary >> 20) % 0x100;
    instClass = (decbits >> 6);
    instClass1 = (decbits >> 5);
    if (decbits == 0x12)
    {
        rn = binary % 0x10;
        // printf("bx%s\t%s\n", ccToStr(cc), regToStr(rn));
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
        // if (accumulate)
        //  printf("mla%s\t%s, %s, %s, %s\n",
        //         ccToStr(cc), regToStr(rd), regToStr(rm), regToStr(rs),
        //         regToStr(rn));
        // else
        //  printf("mul%s\t%s, %s, %s\n",
        //         ccToStr(cc), regToStr(rd), regToStr(rm), regToStr(rs));
    }
    else if (instClass1 == 0x4)
    {
        unsigned up, pre, regList, regsToSaveCount, offset, tmpOffset, i;
        unsigned regsToSave[REG_NUM];

        // LDM and STM
        rn = getRn(binary);
        up = (0x1 & (decbits >> 3));
        pre = (0x1 & (decbits >> 4));
        regList = binary & 0xffff;
        regsToSaveCount = 0;

        for (i = 0; i < REG_NUM; i++)
        {
            if (0x1 & (regList >> i))
            {
                regsToSave[i] = TRUE;
                regsToSaveCount++;
            }
            else
            {
                regsToSave[i] = FALSE;
            }
        }

        offset = regsToSaveCount * 4;

        if (pre)
        {
            if (up)
            {
                R[rn] += offset;
            }
            else
            {
                R[rn] -= offset;
            }

            tmpOffset = 0;

            // Load/store block
            if ((decbits & 0x1) == 0x1)
            {
                // Load block from memory
                for (i = 0; i < REG_NUM; i++)
                {
                    if (regsToSave[i])
                    {
                        R[i] = loadWordFromMem(R[rn] + tmpOffset);
                        tmpOffset += 4;
                    }
                }
            }
            else
            {
                // Store block to memory
                for (i = 0; i < REG_NUM; i++)
                {
                    if (regsToSave[i])
                    {
                        storeWordToMem(R[rn] + tmpOffset, R[i]);
                        tmpOffset += 4;
                    }
                }
            }
        }
        else
        {
            tmpOffset = 0;

            // Load/store block
            if ((decbits & 0x1) == 0x1)
            {
                // Load block from memory
                for (i = 0; i < REG_NUM; i++)
                {
                    if (regsToSave[i])
                    {
                        R[i] = loadWordFromMem(R[rn] + tmpOffset);
                        tmpOffset += 4;
                    }
                }
            }
            else
            {
                // Store block to memory
                for (i = 0; i < REG_NUM; i++)
                {
                    if (regsToSave[i])
                    {
                        storeWordToMem(R[rn] + tmpOffset, R[i]);
                        tmpOffset += 4;
                    }
                }
            }
            if (up)
            {
                R[rn] += offset;
            }
            else
            {
                R[rn] -= offset;
            }
        }
        // printf("stm%s\t%s,%x\n", ccToStr(cc), regToStr(rn), (binary %
        // 0x10000));
    }
    else if (instClass1 == 0x5)
    {
        executeBranchInst(binary);
    }
    else if (instClass == 0x0)
    {
        executeDataProcessingInst(binary);
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
            {
                // printf("ldr%sb\t%s, [%s, #%d]\n",
                //       ccToStr(cc), regToStr(rd), regToStr(rn), offset);
                // FIXME: This should be byte load.
                R[rd] = M[R[rn] + offset];
            }
            else
            {
                // printf("ldr%s\t%s, [%s, #%d]\n",
                //       ccToStr(cc), regToStr(rd), regToStr(rn), offset);
                R[rd] = M[R[rn] + offset];
            }
        }
        else
        {
            // Store to memory instruction
            if (transfB)
            {
                // printf("str%sb\t%s, [%s, #%d]\n",
                //       ccToStr(cc), regToStr(rd), regToStr(rn), offset);
                // FIXME: This should be byte store.
                M[R[rn] + offset] = R[rd];
            }
            else
            {
                // printf("str%s\t%s, [%s, #%d]\n",
                //       ccToStr(cc), regToStr(rd), regToStr(rn), offset);
                M[R[rn] + offset] = R[rd];
            }
        }
    }
}

unsigned fetch()
{
    unsigned inst, PC;

    PC = R[ARM_REG_PC];

    // printf("PC = %d :: binary = 0x%x\n", R[ARM_REG_PC], binary);
    inst = M[PC++];
    inst |= (M[PC++] << 8);
    inst |= (M[PC++] << 16);
    inst |= (M[PC++] << 24);

    R[ARM_REG_PC] = PC;

    return inst;
}

void interpret()
{
    unsigned inst;

    for (;;)
    {
        inst = fetch();
        execute(inst);
        if (R[ARM_REG_PC] > exeheader.codeSize)
            break;
    }
}

int main(int argc, char **argv)
{
    int n, i;
    char *filename;
    FILE *fp;

    filename = argv[1];
    fp = fopen(filename, "rb");

    if (fp == NULL)
    {
        printf("Filename read error.\n");
        exit(1);
    }

    for (i = 0; i < MEMSIZE; i++)
        M[i] = 0x0;

    for (i = 0; i < REG_NUM; i++)
        R[i] = 0x0;

    // Read the file header.
    fread(&exeheader, sizeof(struct objheader_t), 1, fp);

    // printf("codeSize == %d\n", exeheader.codeSize);
    // Read the code
    n = fread(&M, sizeof(char), exeheader.codeSize, fp);
    fclose(fp);

    R[ARM_REG_SP] = MEMSIZE - 1024;
    R[ARM_REG_PC] = exeheader.mainPC;
    R[ARM_REG_LR] = exeheader.codeSize * 2;

    printf("\nHeader data:\n\n");
    printf("  Flags:                    0x%x\n", exeheader.hflags);
    printf("  Code size:                %d\n", exeheader.codeSize);
    printf("  Read only data size:      %d\n", exeheader.readOnlyDataSize);
    printf("  Static data size:         %d\n", exeheader.staticDataSize);
    printf("  Main PC:                  %d\n\n", exeheader.mainPC);

    if (n > 0)
        interpret();
    else
        exit(1);

    return R[0];
}
