// ARM code generator - implementation file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "../include/ARM.h"
#include "../include/ARMCodeGenerator.h"
#include "../include/ARMDecoder.h"
#include "../include/CodeGenerator.h"
#include "../include/Decoder.h"
#include "../include/ELF.h"

//
// Register allocator
//

// TODO: - Create list of permanently allocated regs and every time a register is
//         allocated or deallocated check this list to not free permanently
//         allocated regs.
//       - Implement ELF support.
//       - Implement 'register variable' support (FIXME: Implemented, and have a
//         bug).
//       - Implement FPU support.

// FIXME: Also make sure that register is not non-temporary.
// false - register is free
// true - register is already allocated
unsigned ARMRegisterAllocator::getReg(void)
{
    unsigned r = ARM_REG_R0;

    while (r <= ARM_REG_R12 && isReservedReg(r))
        r++;

    usedRegs |= (0x1 << r);
    return r;
}

bool ARMRegisterAllocator::isReservedReg(unsigned r)
{
    return (((0x1 << r) & usedRegs) != 0x0);
}

void ARMRegisterAllocator::reserveReg(unsigned r)
{
    // assert(!isReservedReg(r) &&
    //       "Trying to reserve already reserved register.");

    usedRegs |= (0x1 << r);
}

void ARMRegisterAllocator::freeReg(unsigned r)
{
    // assert((isReservedReg(r) || isReservedNonTmpReg(r)) &&
    //     "Reserved register expected.");

    if (!isReservedNonTmpReg(r))
        usedRegs &= ~(0x1 << r);
}

unsigned ARMRegisterAllocator::getNonTmpReg(void)
{
    unsigned r = ARM_REG_R0;

    while (r <= ARM_REG_R12 && isReservedReg(r))
        r++;

    usedRegs |= (0x1 << r);
    reserveNonTmpReg(r);
    return r;
}

bool ARMRegisterAllocator::isReservedNonTmpReg(unsigned r)
{
    return (((0x1 << r) & nonTmpRegs) != 0x0);
}

void ARMRegisterAllocator::reserveNonTmpReg(unsigned r)
{
    assert(!isReservedNonTmpReg(r) &&
           "Trying to reserve already reserved non-temporary register.");

    nonTmpRegs |= (0x1 << r);
}

void ARMRegisterAllocator::freeNonTmpReg(unsigned r)
{
    assert(isReservedNonTmpReg(r) &&
           "Reserved non-temporary register expected.");

    nonTmpRegs &= ~(0x1 << r);
}

unsigned ARMRegisterAllocator::getFPUReg(void)
{
    return 0;
}

bool ARMRegisterAllocator::isReservedFPUReg(unsigned f)
{
    return (((0x1 << f) & usedFPURegs) != 0x0);
}

void ARMRegisterAllocator::reserveFPUReg(unsigned f)
{
    assert(!isReservedFPUReg(f) &&
           "Trying to reserve already reserved FPU register.");

    usedFPURegs |= (0x1 << f);
}

void ARMRegisterAllocator::freeFPUReg(unsigned f)
{
    assert(isReservedFPUReg(f) && "Reserved FPU register expected.");

    usedFPURegs &= ~(0x1 << f);
}

unsigned ARMRegisterAllocator::getUsedRegs() { return usedRegs; }

ARMRegisterAllocator::ARMRegisterAllocator()
{
    usedRegs = 0x0;
    nonTmpRegs = 0x0;
}

ARMRegisterAllocator::~ARMRegisterAllocator()
{
    //
}

//
// Code generator
//

enum ARMFixupKind
{
    fixup_ARM_CALL,
    fixup_ARM_JUMP24,
};

//
//   A R M
//

//
// Methods for binary code emission
//

//  [31 | ... | 3 | 2 | 1 | 0]

//#define PUT_IN_BINARY(binary, value, at, numbits) \
//  binary |= ((value << at) & (0x1 << (at + numbits)))

// Data processing instruction format - operand2 is immediate.
#define PUT_DPI_IMM(cond, opcode, rd, rn, operand2)                            \
    putDPInst(cond, opcode, 1, rd, rn, 0, 0, 0, operand2)

// Data processing instruction format - operand2 is register.
#define PUT_DPI_REG(cond, opcode, rd, rn, operand2)                            \
    putDPInst(cond, opcode, 0, rd, rn, 0, 0, 0, operand2)

#define PUT_DPI_REG_SHLL(cond, opcode, rd, rn, operand2, shiftamount)          \
    putDPInst(cond, opcode, 0, rd, rn, 0, 0, shiftamount, operand2)

#define PUT_DPI_IMM_SHIFT(cond, opcode, rd, rn, operand2)                      \
    putDPInst(cond, opcode, 1, rd, rn, operand2)

#define PUT_DPI_REG_SHIFT(cond, opcode, rd, rn, operand2)                      \
    putDPInst(cond, opcode, 0, rd, rn, operand2)

void ARMCodeGenerator::putDPInst(uint8_t cond,
                                 uint8_t opcode,
                                 uint8_t I,
                                 uint8_t rd,
                                 uint8_t rn,
                                 uint8_t shiftIsreg,
                                 uint8_t shiftType,
                                 uint8_t shiftAmountOrRs,
                                 unsigned operand2)
{
    unsigned binary = 0x0;

    setBits(binary, cond, 28, 4);   // binary{31-28} = cond/
                                    // binary{27-26} = 0x0
    setBits(binary, I, 25, 1);      // binary{20-16} = rt
    setBits(binary, opcode, 21, 4); // binary{15-11} = rd
    setBits(binary, rn, 16, 4);     // binary{10-6}  = sa
    setBits(binary, rd, 12, 4);     // binary{5-0}   = opcode
    if (I == 0)
    {
        if (shiftIsreg)
            setBits(binary, shiftAmountOrRs, 8, 4); // binary{11-8} = opcode
        else
            setBits(binary, shiftAmountOrRs, 7, 5); // binary{11-7} = opcode
        setBits(binary, operand2, 0, 4);            // binary{3-0}  = opcode
    }
    else
    {
        setBits(binary, operand2, 0, 8); // binary{7-0} = opcode
    }

    put4b(binary);
}

#define PUT_LOAD_WORD_IMM(cond, rd, rn, offset)                                \
    putSDTInst(cond, 1, 1, 1, 0, 0, 1, rn, rd, offset)

#define PUT_STORE_WORD_IMM(cond, rd, rn, offset)                               \
    putSDTInst(cond, 1, 1, 1, 0, 0, 0, rn, rd, offset)

#define PUT_LOAD_BYTE_IMM(cond, rd, rn, offset)                                \
    putSDTInst(cond, 1, 1, 1, 1, 0, 1, rn, rd, offset)

#define PUT_STORE_BYTE_IMM(cond, rd, rn, offset)                               \
    putSDTInst(cond, 1, 1, 1, 1, 0, 0, rn, rd, offset)

// Single Data Transfer Instructions
void ARMCodeGenerator::putSDTInst(uint8_t cond,
                                  uint8_t I,
                                  uint8_t P,
                                  uint8_t U,
                                  uint8_t B,
                                  uint8_t W,
                                  uint8_t L,
                                  uint8_t rn,
                                  uint8_t rd,
                                  unsigned offset)
{
    unsigned binary = 0x0;

    setBits(binary, cond, 28, 4);   // binary{31-28} = cond
                                    // binary{27-26} = 0x0
    setBits(binary, 0x1, 26, 2);    // binary{20-16} = rt
    setBits(binary, I, 25, 1);      // binary{15-11} = rd
    setBits(binary, P, 24, 1);      // binary{15-11} = rd
    setBits(binary, U, 23, 1);      // binary{15-11} = rd
    setBits(binary, B, 22, 1);      // binary{15-11} = rd
    setBits(binary, W, 21, 1);      // binary{15-11} = rd
    setBits(binary, L, 20, 1);      // binary{15-11} = rd
    setBits(binary, rn, 16, 4);     // binary{10-6}  = sa
    setBits(binary, rd, 12, 4);     // binary{5-0}   = opcode
    setBits(binary, offset, 0, 12); // binary{5-0}   = opcode

    put4b(binary);
}

#define PUT_LDM_PRE(cond, rn, regList)                                         \
    putBDTInst(cond, 1, 1, 0, 0, 1, rn, regList)

#define PUT_LDM_POST(cond, rn, regList)                                        \
    putBDTInst(cond, 0, 1, 0, 0, 1, rn, regList)

#define PUT_STM_PRE(cond, rn, regList)                                         \
    putBDTInst(cond, 1, 1, 0, 0, 0, rn, regList)

#define PUT_STM_POST(cond, rn, regList)                                        \
    putBDTInst(cond, 0, 1, 0, 0, 0, rn, regList)

// Block Data Transfer Instructions
void ARMCodeGenerator::putBDTInst(uint8_t cond,
                                  uint8_t P,
                                  uint8_t U,
                                  uint8_t S,
                                  uint8_t W,
                                  uint8_t L,
                                  uint8_t rn,
                                  unsigned regList)
{
    unsigned binary = 0x0;

    setBits(binary, cond, 28, 4);    // Condition field
    setBits(binary, 0x4, 25, 3);
    setBits(binary, P, 24, 1);       // Pre/Post indexing bit
    setBits(binary, U, 23, 1);       // Up/Down bit
    setBits(binary, S, 22, 1);       // PSR & force user bit
    setBits(binary, W, 21, 1);       // Write-back bit
    setBits(binary, L, 20, 1);       // Load/Store bit
    setBits(binary, rn, 16, 4);      // Base register
    setBits(binary, regList, 0, 16); // Register list

    put4b(binary);
}

// Branch and Branch with Link Instruction
void ARMCodeGenerator::putBranchInst(uint8_t cond, uint8_t L, unsigned offset)
{
    unsigned binary = 0x0;

    setBits(binary, cond, 28, 4);   // Condition field
    setBits(binary, 0x5, 25, 3);
    setBits(binary, L, 24, 1);      // Link bit
    setBits(binary, offset, 0, 24); // Register list

    put4b(binary);
}

// Branch and Exchange Instruction
void ARMCodeGenerator::putBXInst(uint8_t cond, uint8_t rn)
{
    unsigned binary = 0x0;

    setBits(binary, cond, 28, 4); // Condition field
    setBits(binary, 0x1, 24, 4);
    setBits(binary, 0x2, 20, 4);
    setBits(binary, 0xf, 16, 4);
    setBits(binary, 0xf, 12, 4);
    setBits(binary, 0xf, 8, 4);
    setBits(binary, 0x1, 4, 4);
    setBits(binary, rn, 0, 4);    // Operand register

    put4b(binary);
}

#define PUT_MUL(cond, rd, rm, rs) putMulInst(cond, 0, 0, rd, 0, rs, rm)

#define PUT_MLA(cond, rd, rm, rs, rn) putMulInst(cond, 0, 0, rd, rn, rs, rm)

// Multiply and Multiply-Accumulate (MUL, MLA)
void ARMCodeGenerator::putMulInst(uint8_t cond,
                                  uint8_t A,
                                  uint8_t S,
                                  uint8_t rd,
                                  uint8_t rn,
                                  uint8_t rs,
                                  uint8_t rm)
{
    unsigned binary = 0x0;

    setBits(binary, cond, 28, 4); // binary{31-28} = cond
                                  // binary{27-22} = 0x0
    setBits(binary, A, 21, 1);    // binary{21}    = Accumulate bit
    setBits(binary, S, 20, 1);    // binary{20}    = S
    setBits(binary, rd, 16, 4);   // binary{19-16} = rd
    setBits(binary, rn, 12, 4);   // binary{15-12} = rn
    setBits(binary, rs, 8, 4);    // binary{11-8}  = rs
    setBits(binary, 0x9, 4, 4);   // binary{7-4}   = 0x9
    setBits(binary, rm, 0, 4);    // binary{3-0}   = rm

    put4b(binary);
}

//  Listing 1: Example prologue for ARM or Thumb-2 (ARMv7)
//
//  push     {r4-r7, lr}           // save LR, R7, R4-R6
//  add      r7, sp, #12           // adjust R7 to point to saved R7
//  push     {r8, r10, r11}        // save remaining GPRs (R8, R10, R11)
//  vstmdb   sp!, {d8-d15}         // save VFP/Advanced SIMD registers D8
//                                 //  (aka S16-S31, Q4-Q7)
//  sub      sp, sp, #36           // allocate space for local storage
//
//  Listing 2 shows an example epilogue that restores the registers saved
//  by the preceding prologue.
//
//  Listing 2: Example epilogue for ARM or Thumb-2 (ARMv7)
//  add      sp, sp, #36           // deallocate space for local storage
//  vldmia   sp!, {d8-d15}         // restore VFP/Advanced SIMD registers
//  pop      {r8, r10, r11}        // restore R8-R11
//  pop      {r4-r7, pc}           // restore R4-R6, saved R7, return to saved
//                                 // LR
//
//
//  NOTE: Declarations are allowed only at beginning of a function.
//
//  int sum(int x, int y)
//  {
//      int a;
//
//      a = x + y;
//      return a;
//  }
//
//  int main()
//  {
//      int p, q, r;
//
//      p = 22;
//      q = 33;
//      r = sum(p, q);
//      return 0;
//  }
//
//  Symbol table addresses relative to SP:
//  +==========+==========+
//  | Name     |  Address |
//  +==========+==========+
//  |    p     |    52    |
//  |    q     |    48    |
//  |    r     |    44    |
//  +----------+----------+
//  |    x     |    8     |
//  |    y     |    4     |
//  |    a     |    0     |
//  +----------+----------+
//
//  FrameSize(main) = sizeof(p) + sizeof(q) + sizeof(r) +
//                    sizeof([t0..t7]) + sizeof([v0,v1]) +
//                    sizeof(RA)
//                  = 56
//
//  FrameSize(sum)  = sizeof(x) + sizeof(y) + sizeof(a) + sizeof(RA)
//                  = 16
//
//  enter main:
//
//  call sum:
//
//      # Adjust SP
//      addiu sp, sp, -16
//
//      # Save return address register.
//      sw ra, 0(sp)
//
//      # Load arguments onto the stack.
//
//      # Load p
//      lw t0, 12(sp)
//      sw t0, -4(sp)
//
//      # Load q
//      lw t0, 8(sp)
//      sw t0, -8(sp)
//
//      jal Adr(sum)
//      nop
//
//  exit main:
//
//      # Restore return address register.
//      lw ra, 0(sp)
//      jr ra
//      addiu sp, sp, 16
//
//  enter sum:
//
//      # Adjust SP
//      addiu sp, sp, -16
//
//      # Don't need to save RA register because sum do not have
//      # a function call.
//
//  exit sum:
//
//      # Adjust SP
//      jr ra
//      addiu sp, sp, 16
//
//
//  --- Stack ---
//
//  72 |       |
//     +=======+
//  71 |       |
//  70 |       |
//  69 |   p   |
//  68 |       |
//     +=======+
//  67 |       |
//  66 |       |
//  65 |   q   |
//  64 |       |
//     +=======+
//  63 |       |
//  62 |       |
//  61 |   r   |
//  60 |       |
//     +=======+
//     |       |
//     | t0-t7 |
//     | v0,v1 |
//     |       |
//     +=======+
//  19 |       |
//  18 |       |
//  17 |  RA   |
//  16 |       | <--- SP (before sum is entered)
//     +=======+
//  15 |       |
//  14 |       |
//  13 |   x   |
//  12 |       |
//     +=======+
//  11 |       |
//  10 |       |
//   9 |   y   |
//   8 |       |
//     +=======+
//   7 |       |
//   6 |       |
//   5 |   a   |
//   4 |       |
//     +=======+
//   3 |       |
//   2 |       |
//   1 |  RA   |
//   0 |       | <--- SP (after sum is entered)
//     +=======+
//     |       |

unsigned ARMCodeGenerator::getRealReg(const char *virtualReg)
{
    if (strcmp(virtualReg, "SP") == 0)
        return ARM_REG_SP;
    else if (strcmp(virtualReg, "FP") == 0)
        return ARM_REG_SP; // FIXME ??
    else if (strcmp(virtualReg, "GP") == 0)
        return ARM_REG_SP; // FIXME ??
    else if (strcmp(virtualReg, "RA") == 0)
        return ARM_REG_LR;
    else if (strcmp(virtualReg, "RV") == 0)
        return ARM_REG_R0;

    return regalloc->getReg();
}

// TODO: Introduce new parameter dstReg and first try to load
// x into that register. If the register is already used then
// choose arbitrary unused register.
void ARMCodeGenerator::load(Item_t x)
{
    unsigned dstReg;

    if (x->isRegister())
        return;

    if (x->isLocal())
    {
        dstReg = regalloc->getReg();

        if (x->getIRType() == IR_i8)
            PUT_LOAD_BYTE_IMM(CC_AL, dstReg, x->getReg(), x->getAdr());
        else
            PUT_LOAD_WORD_IMM(CC_AL, dstReg, x->getReg(), x->getAdr());

        x->setReg(dstReg);
    }
    else if (x->isStatic())
    {
        // Global variables are stored in data section.

        dstReg = regalloc->getReg();
        // TODO: Implement
        x->setReg(dstReg);
    }
    else if (x->isImm())
    {
        if (TEST_SIMM16_RANGE(x->getImm()))
        {
            dstReg = regalloc->getReg();
            PUT_DPI_IMM(CC_AL, ARM_MOV, dstReg, 0, x->getImm());
            x->setReg(dstReg);
        }
        else
        {
            dstReg = regalloc->getReg();
            // TODO: Implement
            x->setReg(dstReg);
        }
    }
    else if (x->isRegInd())
    {
        dstReg = regalloc->getReg();
        // TODO: Implement
        regalloc->freeReg(x->getReg());
        x->setReg(dstReg);
    }
    else if (x->isAddress())
    {
        // Load absolute address into destination register.
        // Absolute address = REG + OFFSET, where REG is SP or GP.
        dstReg = regalloc->getReg();
        // TODO: Implement
        x->setReg(dstReg);
    }
    x->setMode(I_REG);
}

void ARMCodeGenerator::loadFP(Item_t x)
{
    if (x->isRegister())
        return;

    if (x->isLocal())
    {
        unsigned dstReg = regalloc->getFPUReg();
        // TODO: Implement
        x->setReg(dstReg);
    }
    else if (x->isImm())
    {
        // TODO: Load constant from the constant pool.
    }
    x->setMode(I_REG);
    x->setFPImm(0.0);
}

// TODO: Move this to CodeGenerator.h and overload in subclasses.
// Returns size of IRType in bytes.
static unsigned IRTypeToSize(IRType irtp)
{
    switch (irtp)
    {
        case IR_i8:
            return 1;
        case IR_i16:
            return 2;
        case IR_i32:
            return 4;
        case IR_i64:
            return 8;
        case IR_f32:
            return 4;
        case IR_f64:
            return 8;
        default:
            return -1;
    }
}

void ARMCodeGenerator::index(Item_t x, Item_t y) // x := x[y]
{
    int size = IRTypeToSize(x->getIRType());

    if (y->isImm())
    {
        x->appendAdr(y->getImm() * size);
    }
    else
    {
        if (!y->isRegister())
            load(y);

        if (size != 1)
        {
            // If size is 1 then do not multiply an offset.
            // Small optimization: If size of type is 2 or 4, then shift offset
            // by 1 or 2 bits respectively.
            if (size == 2)
            {
                // TODO: Implement
            }
            else if (size == 4)
            {
                // TODO: Implement
            }
            else
            {
                // Effective address = index * size + adr(array) + SP
                Item_t z = itempool->createItem(I_CONST);

                z->setImm(size);
                load(z);
                // TODO: Implement
                regalloc->freeReg(z->getReg());
            }
        }
        if (x->isLocal())
        {
            x->setReg(y->getReg());
            x->setMode(I_REGIND);
        }
        else if (x->isRegInd())
        {
            // TODO: Implement
        }
    }
}

void ARMCodeGenerator::field(Item_t x, Object_t obj) // x := x.y
{
    if (x->isLocal() || x->isRegInd())
        x->appendAdr(obj->adr);
}

void ARMCodeGenerator::store(Item_t x, Item_t y) // x := y
{
    unsigned opcode;

    if (!y->isRegister())
        load(y);

    if (x->isLocal())
    {
        // TODO: Implement
        if (x->getIRType() == IR_i8)
            PUT_STORE_BYTE_IMM(CC_AL, y->getReg(), x->getReg(), x->getAdr());
        else
            PUT_STORE_WORD_IMM(CC_AL, y->getReg(), x->getReg(), x->getAdr());
        regalloc->freeReg(y->getReg());
        y->setMode(I_LOCAL);
        y->setAdr(x->getAdr());
        y->setReg(x->getReg());
    }
    else if (x->isStatic())
    {
        // Global variables are stored in data section.

        // TODO: Implement
    }
    else if (x->isRegister())
    {
        // TODO: Implement
    }
    else if (x->isRegInd())
    {
        // TODO: Implement
        regalloc->freeReg(x->getReg());
        regalloc->freeReg(y->getReg());
    }
}

void ARMCodeGenerator::storeFP(Item_t x, Item_t y) // x := y
{
    if (!y->isRegister())
        load(y);

    if (x->isLocal())
    {
        // TODO: Implement
        regalloc->freeFPUReg(y->getReg());
        y->setMode(I_LOCAL);
        y->setAdr(x->getAdr());
        y->setReg(x->getReg());
    }
}

void ARMCodeGenerator::inc(Item_t x)
{
    // TODO: Implement
}

void ARMCodeGenerator::unaryOp(GenOp op, Item_t x)
{
    assert(op == GEN_NOT && "unknown unary operation");

    if (x->isImm())
    {
        x->setUImm32(~(x->getUImm32()));
    }
    else
    {
        load(x);
        unsigned dstReg = getTmpReg(x);
        PUT_DPI_REG(CC_AL, ARM_MVN, dstReg, 0, x->getReg());
        x->setReg(dstReg);
    }
}

// This function serves to get temporary register that will be used as
// destination register of binary operation. Idea is to make binary operations
// to look like this:
//   x := x op y
// but we don't want x to by non-temporary allocated register.
// If x is non-temporary then allocate new temporary register instead to use x:
//   z := x op y
//
// Temporary registers are used to hold intermediate results, after usage they
// can be freely modified or deallocated.
// At the other hand the non-temporary registers are bound to a variables,
// and they cannot be used to hold intermediate results, and they cannot be
// modified at all until they are destination registers of store procedure.
unsigned ARMCodeGenerator::getTmpReg(Item_t x)
{
    if (x->getIsTemporary())
        return x->getReg();

    return regalloc->getReg();
}

// x := x op y
void ARMCodeGenerator::binaryOp(GenOp op, Item_t x, Item_t y)
{
    int n;
    unsigned dstReg; // Destination register

    n = 0;

    switch (op)
    {
        case GEN_ADD:
            if (x->isImm() && TEST_SIMM16_RANGE(x->getImm()))
            {
                // y->mode != I_CONST
                load(y);
                dstReg = getTmpReg(y);
                PUT_DPI_IMM(CC_AL, ARM_ADD, dstReg, y->getReg(), x->getImm());
            }
            else if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
            {
                load(x);
                dstReg = getTmpReg(x);
                PUT_DPI_IMM(CC_AL, ARM_ADD, dstReg, x->getReg(), y->getImm());
            }
            else
            {
                load(x);
                load(y);
                dstReg = getTmpReg(x);
                PUT_DPI_REG(CC_AL, ARM_ADD, dstReg, x->getReg(), y->getReg());
                regalloc->freeReg(y->getReg());
            }
            x->setTmpReg(dstReg);
            break;

        case GEN_SUB:
            if (x->isImm() && TEST_SIMM16_RANGE(x->getImm()))
            {
                // y->mode != I_CONST
                load(y);
                dstReg = getTmpReg(y);
                PUT_DPI_IMM(CC_AL, ARM_RSB, dstReg, y->getReg(), x->getImm());
            }
            else if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
            {
                load(x);
                dstReg = getTmpReg(x);
                PUT_DPI_IMM(CC_AL, ARM_SUB, dstReg, x->getReg(), y->getImm());
            }
            else
            {
                load(x);
                load(y);
                dstReg = getTmpReg(x);
                PUT_DPI_REG(CC_AL, ARM_SUB, dstReg, x->getReg(), y->getReg());
                regalloc->freeReg(y->getReg());
            }
            x->setTmpReg(dstReg);
            break;

        case GEN_MUL:
            if (x->isImm(2))
            {
                load(y);
                dstReg = getTmpReg(y);
                PUT_DPI_REG(CC_AL, ARM_ADD, dstReg, y->getReg(), y->getReg());
            }
            else if (x->isImm(4))
            {
                load(y);
                dstReg = getTmpReg(y);
                // TODO: Implement
            }
            else if (x->isImm() && (n = shiftBy(x->getImm())))
            {
                load(y);
                dstReg = getTmpReg(y);
                // TODO: Implement
            }
            else if (y->isImm(2))
            {
                load(x);
                dstReg = getTmpReg(x);
                PUT_DPI_REG(CC_AL, ARM_ADD, dstReg, x->getReg(), x->getReg());
            }
            else if (y->isImm(4))
            {
                load(x);
                dstReg = getTmpReg(x);
                // TODO: Implement
            }
            else if (y->isImm() && (n = shiftBy(y->getImm())))
            {
                load(x);
                dstReg = getTmpReg(x);
                // TODO: Implement
            }
            else
            {
                load(x);
                load(y);
                dstReg = getTmpReg(x);
                PUT_MUL(CC_AL, dstReg, x->getReg(), y->getReg());
                regalloc->freeReg(y->getReg());
            }
            x->setTmpReg(dstReg);
            break;

        case GEN_DIV:
            if (y->isImm(2))
            {
                load(x);
                dstReg = getTmpReg(x);
                // TODO: Implement
            }
            else if (y->isImm(4))
            {
                load(x);
                dstReg = getTmpReg(x);
                // TODO: Implement
            }
            else if (y->isImm() && (n = shiftBy(y->getImm())))
            {
                load(x);
                dstReg = getTmpReg(x);
                // TODO: Implement
            }
            else
            {
                load(x);
                load(y);
                // TODO: Implement
                regalloc->freeReg(y->getReg());
                dstReg = getTmpReg(x);
                // TODO: Implement
            }
            x->setTmpReg(dstReg);
            break;

        case GEN_MOD:
            load(x);
            load(y);
            // TODO: Implement
            regalloc->freeReg(y->getReg());
            dstReg = getTmpReg(x);
            // TODO: Implement
            x->setTmpReg(dstReg);
            break;

        case GEN_AND:
            if (x->isImm() && TEST_SIMM16_RANGE(x->getImm()))
            {
                // y->mode != I_CONST
                load(y);
                dstReg = getTmpReg(y);
                PUT_DPI_IMM(CC_AL, ARM_AND, dstReg, y->getReg(), x->getImm());
            }
            else if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
            {
                load(x);
                dstReg = getTmpReg(x);
                PUT_DPI_IMM(CC_AL, ARM_AND, dstReg, x->getReg(), y->getImm());
            }
            else
            {
                load(x);
                load(y);
                dstReg = getTmpReg(x);
                PUT_DPI_REG(CC_AL, ARM_AND, dstReg, x->getReg(), y->getReg());
                regalloc->freeReg(y->getReg());
            }
            x->setTmpReg(dstReg);
            break;

        case GEN_XOR:
            if (x->isImm() && TEST_SIMM16_RANGE(x->getImm()))
            {
                // y->mode != I_CONST
                load(y);
                dstReg = getTmpReg(y);
                PUT_DPI_IMM(CC_AL, ARM_EOR, dstReg, y->getReg(), x->getImm());
            }
            else if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
            {
                load(x);
                dstReg = getTmpReg(x);
                PUT_DPI_IMM(CC_AL, ARM_EOR, dstReg, x->getReg(), y->getImm());
            }
            else
            {
                load(x);
                load(y);
                dstReg = getTmpReg(x);
                PUT_DPI_REG(CC_AL, ARM_EOR, dstReg, x->getReg(), y->getReg());
                regalloc->freeReg(y->getReg());
            }
            x->setTmpReg(dstReg);
            break;

        case GEN_OR:
            if (x->isImm() && TEST_SIMM16_RANGE(x->getImm()))
            {
                // y->mode != I_CONST
                load(y);
                dstReg = getTmpReg(y);
                PUT_DPI_IMM(CC_AL, ARM_ORR, dstReg, y->getReg(), x->getImm());
            }
            else if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
            {
                load(x);
                dstReg = getTmpReg(x);
                PUT_DPI_IMM(CC_AL, ARM_ORR, dstReg, x->getReg(), y->getImm());
            }
            else
            {
                load(x);
                load(y);
                dstReg = getTmpReg(x);
                PUT_DPI_REG(CC_AL, ARM_ORR, dstReg, x->getReg(), y->getReg());
                regalloc->freeReg(y->getReg());
            }
            x->setTmpReg(dstReg);
            break;
        case GEN_SHL:
            if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
            {
                load(x);
                dstReg = getTmpReg(x);
                PUT_DPI_REG_SHLL(CC_AL, ARM_MOV, dstReg, 0, x->getReg(),
                                 y->getImm());
            }
            else
            {
                load(x);
                load(y);
                dstReg = getTmpReg(x);
                regalloc->freeReg(y->getReg());
            }
            x->setTmpReg(dstReg);
            break;
        case GEN_SHR:
            if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
            {
                load(x);
                dstReg = getTmpReg(x);
                // TODO: Implement
            }
            else
            {
                load(x);
                load(y);
                dstReg = getTmpReg(x);
                // TODO: Implement
                regalloc->freeReg(y->getReg());
            }
            x->setTmpReg(dstReg);
            break;
        case GEN_SHRA:
            if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
            {
                load(x);
                dstReg = getTmpReg(x);
                // TODO: Implement
            }
            else
            {
                load(x);
                load(y);
                dstReg = getTmpReg(x);
                // TODO: Implement
                regalloc->freeReg(y->getReg());
            }
            x->setTmpReg(dstReg);
            break;
        default:
            assert(0 && "unknown binary operation");
    }
}

void ARMCodeGenerator::binaryOp(GenOp op, Item_t x, Item_t y, Item_t z)
{
    switch (op)
    {
        case GEN_ADD:
            if (y->isImm() && z->isImm())
            {
                x->setImm(y->getImm() + z->getImm());
                x->setMode(I_CONST);
            }
            else
            {
                if (z->isImm() && TEST_SIMM16_RANGE(z->getImm()))
                {
                    // y->mode != I_CONST
                    load(y);
                    PUT_DPI_IMM(CC_AL, ARM_ADD, x->getReg(), y->getReg(),
                                z->getImm());
                }
                else if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
                {
                    load(x);
                    PUT_DPI_IMM(CC_AL, ARM_ADD, x->getReg(), z->getReg(),
                                y->getImm());
                }
                else
                {
                    load(y);
                    load(z);
                    PUT_DPI_REG(CC_AL, ARM_ADD, x->getReg(), y->getReg(),
                                z->getReg());
                    if (x->getReg() != y->getReg())
                        regalloc->freeReg(y->getReg());
                    if (x->getReg() != z->getReg())
                        regalloc->freeReg(z->getReg());
                }
            }
            break;
        case GEN_SUB:
            if (y->isImm() && z->isImm())
            {
                // TODO: Move this to the IRExprTree.
                x->setImm(y->getImm() - z->getImm());
                x->setMode(I_CONST);
            }
            else
            {
                if (z->isImm() && TEST_SIMM16_RANGE(-(z->getImm())))
                {
                    // y->mode != I_CONST
                    load(y);
                    PUT_DPI_IMM(CC_AL, ARM_SUB, x->getReg(), y->getReg(),
                                z->getImm());
                }
                else
                {
                    load(y);
                    load(z);
                    PUT_DPI_REG(CC_AL, ARM_SUB, x->getReg(), y->getReg(),
                                z->getReg());
                    if (x->getReg() != y->getReg())
                        regalloc->freeReg(y->getReg());
                    if (x->getReg() != z->getReg())
                        regalloc->freeReg(z->getReg());
                }
            }
            break;
        default:
            assert(0 && "unknown binary operation");
    }
}

void ARMCodeGenerator::binaryOpFP(GenOp op, Item_t x, Item_t y)
{
    int n = 0;
    unsigned dstReg; // Destination register

    switch (op)
    {
        case GEN_ADD:
            if (x->isImm() && y->isImm())
            {
                x->setFPImm(x->getFPImm() + y->getFPImm());
            }
            else
            {
                loadFP(x);
                loadFP(y);
                // TODO: Implement
                regalloc->freeFPUReg(y->getReg());
            }
            break;

        case GEN_SUB:
            break;

        case GEN_MUL:
            break;

        case GEN_DIV:
            break;

        case GEN_MOD:
        case GEN_AND:
        case GEN_XOR:
        case GEN_OR:
        case GEN_SHL:
        case GEN_SHR:
            assert(0 && "invalid binary FPU operation");

        default:
            assert(0 && "unknown binary operation");
    }
}

void ARMCodeGenerator::call(Item_t x,
                            bool preserveReturnValue,
                            const char *fnName)
{
    putBranchInst(CC_AL, 1, getCallTarget(fnName));

    x->setMode(I_REG);

    // Copy return value of the previously called function.
    if (preserveReturnValue)
    {
        x->setReg(regalloc->getReg());
        // FIXME: regalloc->getReg() returns r0 so we get:
        // mov r0, r0
        // Return values are preserved in registers r0-r3.
        PUT_DPI_REG(CC_AL, ARM_MOV, x->getReg(), 0, ARM_REG_R0);
    }
}

void ARMCodeGenerator::callReg(Item_t x,
                               bool preserveReturnValue,
                               const char *fnName)
{
    load(x);
    // TODO: Implement
    x->setMode(I_REG);

    // Copy return value of the previously called function.
    if (preserveReturnValue)
    {
        x->setReg(regalloc->getReg());
        // TODO: Implement
    }
}

// Function entry setup
void ARMCodeGenerator::emitPrologue(int frameSize,
                                    unsigned paramCount,
                                    bool hasCall,
                                    SymbolTable *stb)
{
    //   Implemented calling convention

    //  - Push registers r4-r11 and lr to the stack.
    //  - Adjust stack pointer (for parameters + local variables)
    //  - If subrutine contain a call, push registers r0-r3 containing
    //  parameters to the stack.
    //  - If subrutine doesn't contain a call, leave values in r0-r3 to be used
    //  from there instead to copy them to the stack.

    // Prologue
    // push     {r4-r7, lr}           // save LR, R7, R4-R6

    // 0100 1111 1111 0000
    // 4    f    f    0

    // Save registers {R4-R11, LR}
    if (hasCall)
        PUT_STM_PRE(CC_AL, ARM_REG_SP, 0x4ff0);

    // Adjust the Stack Pointer
    if (frameSize > 0)
        PUT_DPI_IMM(CC_AL, ARM_SUB, ARM_REG_SP, ARM_REG_SP, frameSize);

    // TODO: If function doesn't have a function call, then arguments in
    //  a0, a1, a2 and a3 shouldn't have to be moved onto stack.
    // Move passed function arguments from a0, a1, a2 and a3 to the stack.
    if (paramCount > 0)
    {
        unsigned prmc = (paramCount <= 4) ? paramCount : 4;
        unsigned argOffset = frameSize - 4;

        if (hasCall)
        {
            unsigned regList = 0xf000;

            /*for (unsigned i = 0; i < prmc; i++)
            {
                moveRegisterToStack(i, argOffset);
                argOffset -= 4;
            }*/

            // Save registers r0-r3 to the stack.
            //  1. prmc == 4 => regList is {R0-R3}
            //  2. prmc == 3 => regList is {R0-R2}
            //  3. prmc == 2 => regList is {R0-R1}
            //  FIXME: This should be a single store instruction.
            //  4. prmc == 1 => regList is {R0}

            if (prmc < 4)
                regList = ((0xf000 << (4 - prmc)) % 0x10000);

            PUT_STM_PRE(CC_AL, ARM_REG_SP, regList);
        }
        else
        {
            for (unsigned i = 0; i < prmc; i++)
            {
                Object_t obj = stb->getParameter(i);

                obj->inRegister = true;
                obj->reg = ARM_REG_R0 + i;
                reserveNonTmpReg(ARM_REG_R0 + i);
            }
        }
    }
}

// Function exit
void ARMCodeGenerator::emitEpilogue(int frameSize,
                                    unsigned paramCount,
                                    bool hasCall)
{
    // Epilogue

    // Back SP to its previous state
    if (frameSize > 0)
        PUT_DPI_IMM(CC_AL, ARM_ADD, ARM_REG_SP, ARM_REG_SP, frameSize);

    if (paramCount > 0)
    {
        unsigned prmc = (paramCount <= 4) ? paramCount : 3;

        for (unsigned i = 0; i < prmc; i++)
            if (!hasCall)
                freeNonTmpReg(ARM_REG_R0 + i);
    }

    // 1000 1111 1111 0000
    // 8    f    f    0

    // Restore registers {R4-R11, PC}
    // PUT_LDM(CC_AL, ARM_REG_SP, 0x8ff0);

    // 0100 1111 1111 0000
    // 4    f    f    0

    // Restore registers {R4-R11, LR}
    if (hasCall)
        PUT_LDM_POST(CC_AL, ARM_REG_SP, 0x4ff0);

    PUT_DPI_REG(CC_AL, ARM_MOV, ARM_REG_PC, 0, ARM_REG_LR);
}

// paramNo >= 0 and paramNo <= 3  :  store x into the register $a{paramNo}
// paramNo > 3                    :  push x onto the stack
void ARMCodeGenerator::push(Item_t x,
                            unsigned stackRelAddress,
                            unsigned paramNo)
{
    load(x);
    if (paramNo >= 0 && paramNo <= 3)
    {
        PUT_DPI_REG(CC_AL, ARM_MOV, (ARM_REG_R0 + paramNo), 0, x->getReg());
    }
    else
    {
        // TODO: Implement
    }
    regalloc->freeReg(x->getReg());
}

void ARMCodeGenerator::moveRegisterToStack(unsigned regNo,
                                           unsigned stackRelAddress)
{
    // TODO: Implement
}

void ARMCodeGenerator::pop(void)
{
    // TODO: Implement
}

void ARMCodeGenerator::storeReturnValue(Item_t x)
{
    // Return value goes into register r0.

    if (x->isImm() && TEST_SIMM12_RANGE(x->getImm()))
    {
        PUT_DPI_IMM(CC_AL, ARM_MOV, ARM_REG_R0, 0, x->getImm());
    }
    else
    {
        load(x);
        if (x->getReg() != ARM_REG_R0)
        {
            PUT_DPI_REG(CC_AL, ARM_MOV, ARM_REG_R0, 0, x->getReg());
            regalloc->freeReg(x->getReg());
        }
    }
}

void ARMCodeGenerator::putChar(Item_t x)
{
    load(x);
    // TODO: Implement
}

void ARMCodeGenerator::putInt(Item_t x)
{
    load(x);
    // TODO: Implement
}

void ARMCodeGenerator::setCond(int op, Item_t x, Item_t y)
{
    unsigned dst;

    switch (op)
    {
        case GEN_EQ:
        case GEN_NE:
            load(x);
            load(y);

            if (op == GEN_EQ)
            {
                // x = x == y
            }
            else
            {
                // x = x != y
            }

            // Release previously reserved registers for x and y
            // regalloc->freeReg(y->getReg());
            break;

        case GEN_LT:
        case GEN_GE:
            if (x->isImm() && y->isImm())
                x->setImm(x->getImm() < y->getImm() ? 1 : 0);
            else
            {
                if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
                {
                    // x->mode != I_CONST
                    load(x);
                    // TODO: Implement
                }
                else
                { // x->mode != I_CONST && y->mode != I_CONST
                    load(x);
                    load(y);
                    // TODO: Implement
                }
                x->setMode(I_REG);
                x->setReg(dst);
            }
            break;

        case GEN_GT:
        case GEN_LE:
            // NOTE: x and y switches the place
            if (x->isImm() && y->isImm())
                x->setImm(y->getImm() < x->getImm() ? 1 : 0);
            else
            {
                if (x->isImm() && TEST_SIMM16_RANGE(x->getImm()))
                {
                    // y->mode != I_CONST
                    load(y);
                    // TODO: Implement
                }
                else
                {
                    load(y);
                    load(x);
                    // TODO: Implement
                }
                x->setReg(dst);
                x->setMode(I_REG);
            }
            break;

        default:
            assert(0 && "unknown relation");
    }
}

void ARMCodeGenerator::compareAndJump(unsigned relop,
                                      Item_t x,
                                      Item_t y,
                                      const char *lab)
{
    if (y->isImm())
    {
        load(x);
        PUT_DPI_IMM(CC_AL, ARM_CMP, x->getReg(), 0, y->getImm());

        putBranchInst(relop, 0, getJumpTarget(lab));

        regalloc->freeReg(x->getReg());
    }
    else
    {
        load(x);
        load(y);

        PUT_DPI_REG(CC_AL, ARM_CMP, x->getReg(), y->getReg(), 0);

        putBranchInst(relop, 0, getJumpTarget(lab));

        regalloc->freeReg(x->getReg());
        regalloc->freeReg(y->getReg());
    }
}

void ARMCodeGenerator::condJump(int op, Item_t x, Item_t y, const char *lab)
{
    switch (op)
    {
        case GEN_EQ:
            compareAndJump(CC_EQ, x, y, lab);
            break;
        case GEN_NE:
            compareAndJump(CC_NE, x, y, lab);
            break;
        case GEN_LT:
            compareAndJump(CC_LS, x, y, lab);
            break;
        case GEN_GE:
            compareAndJump(CC_GE, x, y, lab);
            break;
        case GEN_GT:
            compareAndJump(CC_GT, x, y, lab);
            break;
        case GEN_LE:
            compareAndJump(CC_LE, x, y, lab);
            break;
        default:
            assert(0 && "Code generator: Unknown relation");
    }
}

void ARMCodeGenerator::jump(const char *lab)
{
    putBranchInst(CC_AL, 0, getJumpTarget(lab));
}

// Convert type of x to type of y.
void ARMCodeGenerator::convert(Item_t x, Item_t y)
{
    // TODO: Implement
}

static bool isUsed(unsigned usedRegs, unsigned r)
{
    return ((usedRegs & (0x1 << r)) != 0);
}

unsigned ARMCodeGenerator::saveUsedRegs()
{
    unsigned usedRegs = regalloc->getUsedRegs();

    // TODO: Implement

    return usedRegs;
}

void ARMCodeGenerator::restoreSavedRegs(unsigned savedRegs)
{
    // TODO: Implement
}

// TODO: Add these two functions 'addLabel' to CodeGenerator.h
void ARMCodeGenerator::addLabel(const char *name,
                                bool isFunction,
                                bool isGlobal,
                                bool isTemporary,
                                unsigned adr)
{
    GenSymbol *symbol = new GenSymbol(name, adr);

    symbol->setIsFunction(isFunction);
    symbol->setIsGlobal(isGlobal);
    symbol->setIsTemporary(isTemporary);
    labels.insert(std::make_pair(name, symbol));
}

void ARMCodeGenerator::addLabel(const char *name,
                                bool isFunction,
                                bool isGlobal,
                                bool isTemporary)
{
    GenSymbol *symbol = new GenSymbol(name, getPC());

    symbol->setIsFunction(isFunction);
    symbol->setIsGlobal(isGlobal);
    symbol->setIsTemporary(isTemporary);
    labels.insert(std::make_pair(name, symbol));
}

unsigned ARMCodeGenerator::getBranchOffset(const char *lab)
{
    LabelMap::iterator it = labels.find(lab);

    if (it != labels.end())
    {
        GenSymbol *tmp = it->second;

        return (tmp->getAdr() - getPC()) / 4;
    }
    else
    {
        addFixup(fixup_ARM_CALL, getPC(), lab);
        return -1;
    }
}

unsigned ARMCodeGenerator::getJumpTarget(const char *lab)
{
    LabelMap::iterator it = labels.find(lab);

    if (it != labels.end())
    {
        GenSymbol *tmp = it->second;

        return (tmp->getAdr() - getPC()) / 4;
    }
    else
    {
        addFixup(fixup_ARM_JUMP24, getPC(), lab);
        return -1;
    }
}

unsigned ARMCodeGenerator::getCallTarget(const char *lab)
{
    addRelocation(R_ARM_CALL, getPC(), lab);
    return -1;
}

unsigned ARMCodeGenerator::getLabelAdr(const char *s)
{
    LabelMap::iterator it = labels.find(s);

    if (it != labels.end())
    {
        GenSymbol *tmp = it->second;

        return tmp->getAdr();
    }
    else
    {
        // TODO: Add fixup
        return -1;
    }
}

void ARMCodeGenerator::addFixup(unsigned fixupKind,
                                unsigned fixupAdr,
                                const char *label)
{
    fixups.push_back(new Fixup(fixupKind, fixupAdr, label));
}

unsigned ARMCodeGenerator::fixupToRelocation(unsigned fixupKind)
{
    switch (fixupKind)
    {
        case fixup_ARM_CALL:
            return R_ARM_CALL;
        case fixup_ARM_JUMP24:
            return R_ARM_JUMP24;
        default:
            assert(0 && "unknown fixup kind");
    }
}

void ARMCodeGenerator::resolveFixups()
{
    for (unsigned i = 0; i < fixups.size(); i++)
    {
        Fixup *fixup = fixups[i];
        unsigned labelAdr = getLabelAdr(fixup->getLabel());

        if (labelAdr != -1)
        {
            unsigned pos = fixup->getAdr();
            unsigned binary = get4b(pos);

            switch (fixup->getKind())
            {
                case fixup_ARM_JUMP24:
                {
                    unsigned offset;

                    binary &= (binary & 0xff000000);
                    // Forcing a signed division because branch offset can be
                    // negative.
                    offset = (int)((labelAdr - pos) / 4);
                    binary |= offset & 0x00ffffff;
                    put4b(pos, binary);
                }
                break;
                default:
                    assert(0 && "unknown fixup type");
            }
        }
        else
        {
            unsigned relocKind = fixupToRelocation(fixup->getKind());

            addRelocation(relocKind, fixup->getAdr(), fixup->getLabel());
        }
    }
}

void ARMCodeGenerator::emitLabel(const char *labelName)
{
    char str[LABEL_SIZE];
    LabelMap::iterator it;
    GenSymbol *symbol;

    if (!asmOutput)
        return;

    it = labels.find(labelName);

    if (it == labels.end())
        return;

    symbol = it->second;

    putAsmStr("\t.align 2");

    if (symbol->getIsGlobal())
    {
        snprintf(str, LABEL_SIZE, "\t.globl %s", labelName);
        putAsmStr(str);
    }

    putAsmStr("\t.set armv7");

    if (symbol->getIsFunction())
    {
        snprintf(str, LABEL_SIZE, "\t.ent %s", labelName);
        putAsmStr(str);

        snprintf(str, LABEL_SIZE, "\t.type %s, @function", labelName);
        putAsmStr(str);
    }

    snprintf(str, LABEL_SIZE, "%s:", labelName);
    putAsmStr(str);
}

void ARMCodeGenerator::decode()
{
    ARMDecoder decoder(&buf[0], PC, &relocs[0], relocs.size(), isLittleEndian);

    // Print the symbol table
    printf("Symbol table section:\n\n");
    for (int i = 0; i < objheader.symbolsc; i++)
    {
        printf("  Symbol %d {\n", i);
        printf("    Name:           %s\n", symbols[i].name);
        printf("    Offset:         %d\n", symbols[i].offset);
        printf("  }\n\n");
    }

    // Print the relocation section
    printf("Relocation section:\n\n");
    for (int i = 0; i < relocs.size(); i++)
    {
        printf("  Relocation %d {\n", i);
        printf("    Type:           %s\n",
               decoder.getRelocName(ELF32_R_TYPE(relocs[i].r_info)));
        printf("    Offset:         %d\n", relocs[i].r_offset);
        // printf("    Value:          %s\n", relocs[i].value);
        printf("  }\n\n");
    }

    // Print the code buffer
    decoder.printBuffer();
}

// Store the code buffer into filename.out file
void ARMCodeGenerator::write(const char *output)
{
    if (asmOutput)
    {
        // ARM specific asm
    }
    else
    {
        elfObject.setEhdrType(ET_REL);
        // TODO: Set ARM specific flags
        elfObject.setEhdrMachine(0x0);
        elfObject.setEhdrVersion(EV_CURRENT);
        elfObject.setEhdrEntry(objheader.mainPC);
        elfObject.setEhdrPhoff(0x0);
        // FIXME: Section header table's file offset in bytes.
        elfObject.setEhdrShoff(sizeof(Elf32_Ehdr));
        elfObject.setEhdrFlags(0x0);
        elfObject.setEhdrEhsize(sizeof(Elf32_Ehdr));
        elfObject.setEhdrPhentsize(0x0);
        elfObject.setEhdrPhnum(0x0);
        elfObject.setEhdrShentsize(sizeof(Elf32_Shdr));
        // FIXME: Here goes number of sections.
        // So far we have 3 sections:
        //   - String table section
        //   - Symbol table section
        //   - Relocations section
        //   - Text section
        elfObject.setEhdrShnum(SHDRTAB_NUM);
        elfObject.setEhdrShstrndx(1);

        elfObject.setTextSection(&buf[0]);
        elfObject.setTextSectionSize(PC);
    }

    CodeGenerator::write(output);
}
