// Intel8086 code generator - implementation file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "../include/CodeGenerator.h"
#include "../include/Decoder.h"
#include "../include/ELF.h"
#include "../include/IRExpr.h"
#include "../include/Intel8086.h"
#include "../include/Intel8086CodeGenerator.h"

//
// Register allocator
//

// TODO: - Create list of permanently allocated regs and every time a register
//         is allocated or deallocated check this list to not free permanently
//         allocated regs.
//       - Implement ELF support.
//       - Implement 'register variable' support (FIXME: Implemented, and have a
//         bug).
//       - Implement FPU support.

// FIXME: Also make sure that register is not non-temporary.
// false - register is free
// true - register is already allocated
unsigned Intel8086RegisterAllocator::getReg(void)
{
    unsigned r = REG_Intel8086_AX;

    while (r <= REG_Intel8086_DX && isReservedReg(r))
        r++;

    usedRegs |= (0x1 << r);
    return r;
}

bool Intel8086RegisterAllocator::isReservedReg(unsigned r)
{
    return (((0x1 << r) & usedRegs) != 0x0);
}

void Intel8086RegisterAllocator::reserveReg(unsigned r)
{
    // assert(!isReservedReg(r) &&
    //       "Trying to reserve already reserved register.");

    usedRegs |= (0x1 << r);
}

void Intel8086RegisterAllocator::freeReg(unsigned r)
{
    // assert((isReservedReg(r) || isReservedNonTmpReg(r)) &&
    //     "Reserved register expected.");

    // if (!isReservedNonTmpReg(r))
    usedRegs &= ~(0x1 << r);
}

unsigned Intel8086RegisterAllocator::getNonTmpReg(void) { return 0; }

bool Intel8086RegisterAllocator::isReservedNonTmpReg(unsigned r)
{
    return (((0x1 << r) & nonTmpRegs) != 0x0);
}

void Intel8086RegisterAllocator::reserveNonTmpReg(unsigned r)
{
    assert(!isReservedNonTmpReg(r) &&
           "Trying to reserve already reserved non-temporary register.");

    nonTmpRegs |= (0x1 << r);
}

void Intel8086RegisterAllocator::freeNonTmpReg(unsigned r)
{
    assert(isReservedNonTmpReg(r) &&
           "Reserved non-temporary register expected.");

    nonTmpRegs &= ~(0x1 << r);
}

bool Intel8086RegisterAllocator::isReservedFPUReg(unsigned f)
{
    return (((0x1 << f) & usedFPURegs) != 0x0);
}

void Intel8086RegisterAllocator::reserveFPUReg(unsigned f)
{
    assert(!isReservedFPUReg(f) &&
           "Trying to reserve already reserved FPU register.");

    usedFPURegs |= (0x1 << f);
}

void Intel8086RegisterAllocator::freeFPUReg(unsigned f)
{
    assert(isReservedFPUReg(f) && "Reserved FPU register expected.");

    usedFPURegs &= ~(0x1 << f);
}

unsigned Intel8086RegisterAllocator::getUsedRegs() { return usedRegs; }

void Intel8086RegisterAllocator::setUsedRegs(unsigned value)
{
    usedRegs = value;
}

Intel8086RegisterAllocator::Intel8086RegisterAllocator()
{
    usedRegs = 0x0;
    nonTmpRegs = 0x0;
}

Intel8086RegisterAllocator::~Intel8086RegisterAllocator()
{
    // TODO: Implement
}

//
// Code generator
//

//
//   Intel 8086
//

static const char *regToStr(unsigned reg)
{
    switch (reg)
    {
        case REG_Intel8086_AX:
            return "ax";
        case REG_Intel8086_BX:
            return "bx";
        case REG_Intel8086_CX:
            return "cx";
        case REG_Intel8086_DX:
            return "dx";
        case REG_Intel8086_AH:
            return "ah";
        case REG_Intel8086_AL:
            return "al";
        case REG_Intel8086_BL:
            return "bl";
        case REG_Intel8086_BH:
            return "bh";
        case REG_Intel8086_CH:
            return "ch";
        case REG_Intel8086_CL:
            return "cl";
        case REG_Intel8086_DH:
            return "dh";
        case REG_Intel8086_DL:
            return "dl";
        case REG_Intel8086_DI:
            return "di";
        case REG_Intel8086_SI:
            return "si";
        case REG_Intel8086_BP:
            return "bp";
        case REG_Intel8086_SP:
            return "sp";

        default:
            assert(0 && "unknown register");
    }
}

const char *Intel8086CodeGenerator::GenIntel8086OpcToAsm(GenIntel8086Opc opcode)
{
    switch (opcode)
    {
        case GEN_Intel8086_AAA:
            return "aaa";
        case GEN_Intel8086_AAD:
            return "aad";
        case GEN_Intel8086_AAM:
            return "aam";
        case GEN_Intel8086_AAS:
            return "aas";
        case GEN_Intel8086_ADC:
            return "adc";
        case GEN_Intel8086_ADD:
            return "add";
        case GEN_Intel8086_AND:
            return "and";
        case GEN_Intel8086_CALL:
            return "call";
        case GEN_Intel8086_CMP:
            return "cmp";
        case GEN_Intel8086_DIV:
            return "div";
        case GEN_Intel8086_INT:
            return "int";
        case GEN_Intel8086_JA:
            return "ja";
        case GEN_Intel8086_JAE:
            return "jae";
        case GEN_Intel8086_JB:
            return "jb";
        case GEN_Intel8086_JBE:
            return "jbe";
        case GEN_Intel8086_JC:
            return "jc";
        case GEN_Intel8086_JCXZ:
            return "jcxz";
        case GEN_Intel8086_JE:
            return "je";
        case GEN_Intel8086_JG:
            return "jg";
        case GEN_Intel8086_JGE:
            return "jge";
        case GEN_Intel8086_JL:
            return "jl";
        case GEN_Intel8086_JLE:
            return "jle";
        case GEN_Intel8086_JMP:
            return "jmp";
        case GEN_Intel8086_JNA:
            return "jna";
        case GEN_Intel8086_JNAE:
            return "jnae";
        case GEN_Intel8086_JNB:
            return "jnb";
        case GEN_Intel8086_JNBE:
            return "jnbe";
        case GEN_Intel8086_JNC:
            return "jnc";
        case GEN_Intel8086_JNE:
            return "jne";
        case GEN_Intel8086_JNG:
            return "jng";
        case GEN_Intel8086_JNGE:
            return "jnge";
        case GEN_Intel8086_JNL:
            return "jnl";
        case GEN_Intel8086_JNLE:
            return "jnle";
        case GEN_Intel8086_JNO:
            return "jno";
        case GEN_Intel8086_JNP:
            return "jnp";
        case GEN_Intel8086_JNS:
            return "jns";
        case GEN_Intel8086_JNZ:
            return "jnz";
        case GEN_Intel8086_JO:
            return "jo";
        case GEN_Intel8086_JP:
            return "jp";
        case GEN_Intel8086_JPE:
            return "jpe";
        case GEN_Intel8086_JPO:
            return "jpo";
        case GEN_Intel8086_JS:
            return "js";
        case GEN_Intel8086_JZ:
            return "jz";
        case GEN_Intel8086_LOOP:
            return "loop";
        case GEN_Intel8086_LOOPE:
            return "loope";
        case GEN_Intel8086_LOOPNE:
            return "loopne";
        case GEN_Intel8086_LOOPNZ:
            return "loopnz";
        case GEN_Intel8086_LOOPZ:
            return "loopz";
        case GEN_Intel8086_MOV:
            return "mov";
        case GEN_Intel8086_MOVSB:
            return "movsb";
        case GEN_Intel8086_MOVSW:
            return "movsw";
        case GEN_Intel8086_MUL:
            return "mul";
        case GEN_Intel8086_NOP:
            return "nop";
        case GEN_Intel8086_NOT:
            return "not";
        case GEN_Intel8086_OR:
            return "or";
        case GEN_Intel8086_POP:
            return "pop";
        case GEN_Intel8086_PUSH:
            return "push";
        case GEN_Intel8086_RET:
            return "ret";
        case GEN_Intel8086_SHL:
            return "shl";
        case GEN_Intel8086_SHR:
            return "shr";
        case GEN_Intel8086_SUB:
            return "sub";
        case GEN_Intel8086_XOR:
            return "xor";

        default:
            assert(0 && "unknown opcode");
    }
}

//
// Methods for assembly code emission
//

void Intel8086CodeGenerator::put_opc(GenIntel8086Opc opcode)
{
    char inst[256];

    sprintf(inst, "\t%s", GenIntel8086OpcToAsm(opcode));
    putAsmStr(inst);
}

void Intel8086CodeGenerator::put_opc_lab(GenIntel8086Opc opcode,
                                         const char *lab)
{
    char inst[256];

    sprintf(inst, "\t%s\t%s", GenIntel8086OpcToAsm(opcode), lab);
    putAsmStr(inst);
}

void Intel8086CodeGenerator::put_opc_imm(GenIntel8086Opc opcode, int imm)
{
    char inst[256];

    sprintf(inst, "\t%s\t%d", GenIntel8086OpcToAsm(opcode), imm);
    putAsmStr(inst);
}

void Intel8086CodeGenerator::put_opc_reg(GenIntel8086Opc opcode, unsigned reg)
{
    char inst[256];

    sprintf(inst, "\t%s\t%s", GenIntel8086OpcToAsm(opcode), regToStr(reg));
    putAsmStr(inst);
}

void Intel8086CodeGenerator::put_opc_reg_reg(GenIntel8086Opc opcode,
                                             unsigned reg1,
                                             unsigned reg2)
{
    char inst[256];

    sprintf(inst, "\t%s\t%s, %s", GenIntel8086OpcToAsm(opcode), regToStr(reg1),
            regToStr(reg2));
    putAsmStr(inst);
}

void Intel8086CodeGenerator::put_opc_reg_imm(GenIntel8086Opc opcode,
                                             unsigned reg1,
                                             int imm)
{
    char inst[256];

    sprintf(inst, "\t%s\t%s, %d", GenIntel8086OpcToAsm(opcode), regToStr(reg1),
            imm);
    putAsmStr(inst);
}

void Intel8086CodeGenerator::put_opc_reg_mem(GenIntel8086Opc opcode,
                                             unsigned reg,
                                             unsigned base,
                                             int offset)
{
    char inst[256];

    sprintf(inst, "\t%s\t%s, [%s+%d]", GenIntel8086OpcToAsm(opcode),
            regToStr(reg), regToStr(base), offset);
    putAsmStr(inst);
}

void Intel8086CodeGenerator::put_opc_mem_reg(GenIntel8086Opc opcode,
                                             unsigned base,
                                             int offset,
                                             unsigned reg)
{
    char inst[256];

    sprintf(inst, "\t%s\t[%s+%d], %s", GenIntel8086OpcToAsm(opcode),
            regToStr(base), offset, regToStr(reg));
    putAsmStr(inst);
}

void Intel8086CodeGenerator::put_opc_mem_imm(GenIntel8086Opc opcode,
                                             unsigned base,
                                             int offset,
                                             int imm)
{
    char inst[256];

    sprintf(inst, "\t%s\t[%s+%d], %d", GenIntel8086OpcToAsm(opcode),
            regToStr(base), offset, imm);
    putAsmStr(inst);
}

bool isLoadStore(GenIntel8086Opc opcode) { return false; }

/*  NOTE: Declarations are allowed only at beginning of a function.

    int sum(int x, int y)
    {
        int a;

        a = x + y;
        return a;
    }

    int main()
    {
        int p, q, r;

        p = 22;
        q = 33;
        r = sum(p, q);
        return 0;
    }

    Symbol table addresses relative to SP:
    +==========+==========+
    | Name     |  Address |
    +==========+==========+
    |    p     |    52    |
    |    q     |    48    |
    |    r     |    44    |
    +----------+----------+
    |    x     |    8     |
    |    y     |    4     |
    |    a     |    0     |
    +----------+----------+

    FrameSize(main) = sizeof(p) + sizeof(q) + sizeof(r) +
                      sizeof([t0..t7]) + sizeof([v0,v1]) +
                      sizeof(RA)
                    = 56

    FrameSize(sum)  = sizeof(x) + sizeof(y) + sizeof(a) + sizeof(RA)
                    = 16
*/

void Intel8086CodeGenerator::addToDataSection(unsigned adr, unsigned value)
{
    data[adr] = value;
}

unsigned Intel8086CodeGenerator::getRealReg(const char *virtualReg)
{
    if (strcmp(virtualReg, "SP") == 0)
        return REG_Intel8086_BP;
    else if (strcmp(virtualReg, "FP") == 0)
        return -1;
    else if (strcmp(virtualReg, "GP") == 0)
        return -1;
    else if (strcmp(virtualReg, "RA") == 0)
        return -1;
    else if (strcmp(virtualReg, "RV") == 0)
        return -1;
    else if (strcmp(virtualReg, "A0") == 0)
        return -1;
    else if (strcmp(virtualReg, "A1") == 0)
        return -1;
    else if (strcmp(virtualReg, "A2") == 0)
        return -1;
    else if (strcmp(virtualReg, "A3") == 0)
        return -1;

    return regalloc->getReg();
}

// TODO: Introduce new parameter dstReg and first try to load x into that
// register. If the register is already used then choose arbitrary unused
// register.
void Intel8086CodeGenerator::load(Item_t x)
{
    unsigned dstReg;

    if (x->isRegister())
        return;

    if (x->isLocal())
    {
        dstReg = regalloc->getReg();
        put_opc_reg(GEN_Intel8086_PUSH, REG_Intel8086_AX);
        put_opc_reg_mem(GEN_Intel8086_MOV, REG_Intel8086_AX, x->getBaseReg(),
                        x->getAdr());
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
            put_opc_reg_imm(GEN_Intel8086_MOV, dstReg, x->getImm());
            x->setReg(dstReg);
        }
        else
        {
            dstReg = regalloc->getReg();
            x->setReg(dstReg);
        }
    }
    else if (x->isRegInd())
    {
        dstReg = regalloc->getReg();
        regalloc->freeReg(x->getReg());
        x->setReg(dstReg);
    }
    else if (x->isAddress())
    {
        // Load absolute address into destination register.
        // Absolute address = REG + OFFSET, where REG is SP or GP.
        dstReg = regalloc->getReg();
        x->setReg(dstReg);
    }
    x->setMode(I_REG);
}

// TODO: Introduce new parameter dstReg and first try to load
// x into that register. If the register is already used then
// choose arbitrary unused register.
void Intel8086CodeGenerator::loadTo(unsigned reg, Item_t x)
{
    unsigned dstReg;

    if (x->isRegister())
    {
        put_opc_reg_reg(GEN_Intel8086_MOV, reg, x->getReg());
    }
    else if (x->isLocal())
    {
        dstReg = regalloc->getReg();
        put_opc_reg_mem(GEN_Intel8086_MOV, reg, x->getBaseReg(), x->getAdr());
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
            put_opc_reg_imm(GEN_Intel8086_MOV, reg, x->getImm());
            x->setReg(dstReg);
        }
        else
        {
            dstReg = regalloc->getReg();
            x->setReg(dstReg);
        }
    }
    else if (x->isRegInd())
    {
        put_opc_reg_mem(GEN_Intel8086_MOV, reg, x->getReg(), x->getAdr());
        x->setReg(dstReg);
    }
    else if (x->isAddress())
    {
        // Load absolute address into destination register.
        // Absolute address = REG + OFFSET, where REG is SP or GP.
        dstReg = regalloc->getReg();
        x->setReg(dstReg);
    }
    x->setMode(I_REG);
}

void Intel8086CodeGenerator::loadMem(Item_t x, Item_t y)
{
    // TODO: Implement
}

void Intel8086CodeGenerator::loadIndRef(Item_t x)
{
    load(x);

    x->setMode(I_REGIND);
}

void Intel8086CodeGenerator::loadLabelRelative(Item_t x,
                                               const char *labelName,
                                               unsigned offset)
{
    unsigned dstReg = regalloc->getReg();

    // TODO: Implement

    x->setReg(dstReg);
}

void Intel8086CodeGenerator::loadArg(Item_t y)
{
    // TODO: Implement
}

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

void Intel8086CodeGenerator::index(Item_t x, Item_t y) // x := x[y]
{
    int size = IRTypeToSize(x->getIRType());

    if (y->isImm())
    {
        x->setAdr(x->getAdr() + y->getImm() * size);
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
                regalloc->freeReg(z->getReg());
            }
        }
        if (x->isLocal())
        {
            // t0 = index * size;
            // ADDU  t0, SP, t0
            // ADDIU t0, adr(arr), t0 */

            x->setReg(y->getReg());
            x->setMode(I_REGIND);
            // x->adr must not be changed! It will be used when x->isRegInd() is
            // detected.
        }
        else if (x->isStatic())
        {
            // Global variables are stored in data section.

            unsigned tmpReg = regalloc->getReg();

            x->setReg(y->getReg());
            x->setMode(I_REGIND);
            // x->adr must not be changed! It will be used when x->isRegInd() is
            // detected.
            regalloc->freeReg(tmpReg);
        }
        else if (x->isRegInd())
        {
            // TODO: Implement
        }
    }
}

void Intel8086CodeGenerator::field(Item_t x, Object_t obj) // x := x.y
{
    if (x->isLocal() || x->isRegInd())
    {
        x->setAdr(x->getAdr() + obj->adr);
    }
}

void Intel8086CodeGenerator::store(Item_t x, Item_t y) // x := y
{
    if (!y->isRegister())
        load(y);

    if (x->isLocal())
    {
        put_opc_mem_reg(GEN_Intel8086_MOV, x->getBaseReg(), x->getAdr(),
                        y->getReg());
        regalloc->freeReg(y->getReg());
    }
    else if (x->isStatic())
    {
        // Global variables are stored in data section.

        unsigned tmpReg = regalloc->getReg();

        regalloc->freeReg(tmpReg);
        regalloc->freeReg(y->getReg());
    }
    else if (x->isRegister())
    {
        regalloc->freeReg(y->getReg());
    }
    else if (x->isRegInd())
    {
        regalloc->freeReg(x->getReg());
        regalloc->freeReg(y->getReg());
    }
}

void Intel8086CodeGenerator::unaryOp(GenOp op, Item_t x)
{
    if (op == GEN_NOT)
    {
        load(x);
        put_opc_reg(GEN_Intel8086_NOT, x->getReg());
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
unsigned Intel8086CodeGenerator::getTmpReg(Item_t x)
{
    assert(x->getMode() == I_REG);

    if (x->getIsTemporary())
        return x->getReg();

    return regalloc->getReg();
}

// x := x op y
void Intel8086CodeGenerator::binaryOp(GenOp op, Item_t x, Item_t y)
{
    int n = 0;
    unsigned dstReg; // Destination register

    switch (op)
    {
        case GEN_ADD:
            if (x->isImm() && TEST_SIMM16_RANGE(x->getImm()))
            {
                // y->mode != I_CONST
                load(y);
                dstReg = getTmpReg(y);
                put_opc_reg_imm(GEN_Intel8086_ADD, y->getReg(), x->getImm());
            }
            else if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
            {
                load(x);
                dstReg = getTmpReg(x);
                put_opc_reg_imm(GEN_Intel8086_ADD, x->getReg(), y->getImm());
            }
            else
            {
                load(x);
                load(y);
                dstReg = getTmpReg(x);
                put_opc_reg_reg(GEN_Intel8086_ADD, x->getReg(), y->getReg());
                regalloc->freeReg(y->getReg());
            }
            x->setTmpReg(dstReg);
            break;

        case GEN_SUB:
            if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
            {
                load(x);
                dstReg = getTmpReg(x);
                put_opc_reg_imm(GEN_Intel8086_SUB, x->getReg(), y->getImm());
                x->setTmpReg(dstReg);
            }
            else
            {
                load(x);
                load(y);
                dstReg = getTmpReg(x);
                put_opc_reg_reg(GEN_Intel8086_SUB, x->getReg(), y->getReg());
                regalloc->freeReg(y->getReg());
                x->setTmpReg(dstReg);
            }
            break;

        case GEN_MUL:
            if (x->isImm(2))
            {
                load(y);
                dstReg = getTmpReg(y);
            }
            else if (x->isImm(4))
            {
                load(y);
                dstReg = getTmpReg(y);
                put_opc_reg_imm(GEN_Intel8086_SHL, y->getReg(), 2);
            }
            else if (x->isImm() && (n = shiftBy(x->getImm())))
            {
                load(y);
                dstReg = getTmpReg(y);
                put_opc_reg_imm(GEN_Intel8086_SHL, y->getReg(), n);
            }
            else if (y->isImm(2))
            {
                load(x);
                dstReg = getTmpReg(x);
                put_opc_reg_reg(GEN_Intel8086_ADD, x->getReg(), x->getReg());
            }
            else if (y->isImm(4))
            {
                load(x);
                dstReg = getTmpReg(x);
                put_opc_reg_imm(GEN_Intel8086_SHL, x->getReg(), 2);
            }
            else if (y->isImm() && (n = shiftBy(y->getImm())))
            {
                load(x);
                dstReg = getTmpReg(x);
            }
            else
            {
                load(x);
                load(y);
                dstReg = getTmpReg(x);
                // FIXME: MUL has single parameter
                put_opc_reg_reg(GEN_Intel8086_MUL, x->getReg(), y->getReg());
                regalloc->freeReg(y->getReg());
            }
            x->setTmpReg(dstReg);
            break;

        case GEN_DIV:
            if (y->isImm(2))
            {
                load(x);
                dstReg = getTmpReg(x);
            }
            else if (y->isImm(4))
            {
                load(x);
                dstReg = getTmpReg(x);
            }
            else if (y->isImm() && (n = shiftBy(y->getImm())))
            {
                load(x);
                dstReg = getTmpReg(x);
            }
            else
            {
                load(x);
                load(y);
                // FIXME: DIV has single parameter
                put_opc_reg_reg(GEN_Intel8086_DIV, x->getReg(), y->getReg());
                regalloc->freeReg(y->getReg());
                dstReg = getTmpReg(x);
            }
            x->setTmpReg(dstReg);
            break;

        case GEN_MOD:
            load(x);
            load(y);
            regalloc->freeReg(y->getReg());
            dstReg = getTmpReg(x);
            x->setTmpReg(dstReg);
            break;

        case GEN_AND:
            if (x->isImm() && TEST_SIMM16_RANGE(x->getImm()))
            {
                // y->mode != I_CONST
                load(y);
                dstReg = getTmpReg(y);
                put_opc_reg_imm(GEN_Intel8086_AND, y->getReg(), x->getImm());
            }
            else if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
            {
                load(x);
                dstReg = getTmpReg(x);
                put_opc_reg_imm(GEN_Intel8086_AND, x->getReg(), y->getImm());
            }
            else
            {
                load(x);
                load(y);
                dstReg = getTmpReg(x);
                put_opc_reg_reg(GEN_Intel8086_AND, x->getReg(), y->getReg());
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
                put_opc_reg_imm(GEN_Intel8086_XOR, y->getReg(), x->getImm());
            }
            else if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
            {
                load(x);
                dstReg = getTmpReg(x);
                put_opc_reg_imm(GEN_Intel8086_XOR, x->getReg(), y->getImm());
            }
            else
            {
                load(x);
                load(y);
                dstReg = getTmpReg(x);
                put_opc_reg_reg(GEN_Intel8086_XOR, x->getReg(), y->getReg());
                regalloc->freeReg(y->getReg());
            }
            x->setTmpReg(dstReg);
            break;

        case GEN_OR:
            if (x->isImm() && TEST_SIMM16_RANGE(x->getImm()))
            {
                /* y->mode != I_CONST */
                load(y);
                dstReg = getTmpReg(y);
                put_opc_reg_imm(GEN_Intel8086_OR, y->getReg(), x->getImm());
            }
            else if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
            {
                load(x);
                dstReg = getTmpReg(x);
                put_opc_reg_imm(GEN_Intel8086_OR, x->getReg(), y->getImm());
            }
            else
            {
                load(x);
                load(y);
                dstReg = getTmpReg(x);
                put_opc_reg_reg(GEN_Intel8086_OR, x->getReg(), y->getReg());
                regalloc->freeReg(y->getReg());
            }
            x->setTmpReg(dstReg);
            break;
        case GEN_SHL:
            if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
            {
                load(x);
                dstReg = getTmpReg(x);
                put_opc_reg_reg(GEN_Intel8086_SHL, x->getReg(), y->getReg());
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
                put_opc_reg_reg(GEN_Intel8086_SHR, x->getReg(), y->getReg());
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
        case GEN_SHRA:
            if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
            {
                load(x);
                dstReg = getTmpReg(x);
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
        default:
            assert(0 && "unknown binary operation");
    }
}

void Intel8086CodeGenerator::binaryOp(GenOp op, Item_t x, Item_t y, Item_t z)
{
    int n = 0;
    unsigned dstReg; // Destination register

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
                }
                else if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
                {
                    load(x);
                }
                else
                {
                    load(y);
                    load(z);
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
                x->setImm(y->getImm() - z->getImm());
                x->setMode(I_CONST);
            }
            else
            {
                if (z->isImm() && TEST_SIMM16_RANGE(-(z->getImm())))
                {
                    // y->mode != I_CONST
                    load(y);
                }
                else
                {
                    load(y);
                    load(z);
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

void Intel8086CodeGenerator::call(Item_t x,
                                  bool preserveReturnValue,
                                  const char *fnName)
{
    // Copy return value of the previously called function.
    if (preserveReturnValue)
    {
        x->setMode(I_REG);
        x->setReg(regalloc->getReg());
        x->setIsTemporary(true);
        put_opc_lab(GEN_Intel8086_CALL, fnName);
    }
}

void Intel8086CodeGenerator::callReg(Item_t x, bool preserveReturnValue)
{
    load(x);

    // Copy return value of the previously called function.
    if (preserveReturnValue)
    {
        x->setMode(I_REG);
        x->setReg(regalloc->getReg());
        x->setIsTemporary(true);
    }
}

/* Function entry setup */
void Intel8086CodeGenerator::emitPrologue(int frameSize,
                                          unsigned paramCount,
                                          bool hasCall,
                                          SymbolTable *stb)
{
    if (frameSize > 0)
    {
        // TODO: Implement
    }
    if (hasCall)
    {
        // TODO: Implement
    }

    put_opc_reg(GEN_Intel8086_PUSH, REG_Intel8086_AX);
    put_opc_reg(GEN_Intel8086_PUSH, REG_Intel8086_BX);
    put_opc_reg(GEN_Intel8086_PUSH, REG_Intel8086_CX);
    put_opc_reg(GEN_Intel8086_PUSH, REG_Intel8086_DX);
    put_opc_reg(GEN_Intel8086_PUSH, REG_Intel8086_BP);
    put_opc_reg_reg(GEN_Intel8086_MOV, REG_Intel8086_BP, REG_Intel8086_SP);
}

void Intel8086CodeGenerator::emitEpilogue(int frameSize,
                                          unsigned paramCount,
                                          bool hasCall)
{
    if (hasCall)
    {
        // TODO: Implement
    }

    put_opc_reg(GEN_Intel8086_POP, REG_Intel8086_BP);
    put_opc_reg(GEN_Intel8086_POP, REG_Intel8086_DX);
    put_opc_reg(GEN_Intel8086_POP, REG_Intel8086_CX);
    put_opc_reg(GEN_Intel8086_POP, REG_Intel8086_BX);
    put_opc_reg(GEN_Intel8086_POP, REG_Intel8086_AX);
}

// paramNo >= 0 and paramNo <= 3  :  store x into the register $a{paramNo}
// paramNo > 3                    :  push x onto the stack
void Intel8086CodeGenerator::push(Item_t x,
                                  unsigned stackRelAddress,
                                  unsigned paramNo)
{
    load(x);

    regalloc->freeReg(x->getReg());
}

void Intel8086CodeGenerator::moveRegisterToStack(unsigned regNo,
                                                 unsigned stackRelAddress)
{
    // TODO: Implement
}

void Intel8086CodeGenerator::pop(void)
{
    // TODO: Implement
}

void Intel8086CodeGenerator::move(Item_t x, Item_t y)
{
    if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
    {
        // TODO: Implement
    }
    else
    {
        load(y);
        regalloc->freeReg(y->getReg());
    }
}

void Intel8086CodeGenerator::storeReturnValue(Item_t x)
{
    if (x->isImm() && TEST_SIMM16_RANGE(x->getImm()))
    {
        // TODO: Implement
    }
    else
    {
        load(x);
        regalloc->freeReg(x->getReg());
    }
}

void Intel8086CodeGenerator::emitReturn()
{
    // put_opc_imm(GEN_Intel8086_RET, 4);
    put_opc(GEN_Intel8086_RET);
}

void Intel8086CodeGenerator::emitAsmEndFunctionDirectives(const char *funcName)
{
    char buf[ASM_STR_SIZE];

    PUT_ASM_1(buf, "%s endp", funcName);
}

/* if x = 0 then y := z */
void Intel8086CodeGenerator::emitCondInst(GenOp op,
                                          Item_t x,
                                          Item_t y,
                                          Item_t z)
{
    // TODO: Implement
}

void Intel8086CodeGenerator::label(Item_t x, const char *name)
{
    // TODO: Implement
}

void Intel8086CodeGenerator::emitGoto(const char *labelName)
{
    // TODO: Implement
}

void Intel8086CodeGenerator::putChar(Item_t x)
{
    put_opc_reg(GEN_Intel8086_PUSH, REG_Intel8086_AX);
    put_opc_reg(GEN_Intel8086_PUSH, REG_Intel8086_DX);
    put_opc_reg_imm(GEN_Intel8086_MOV, REG_Intel8086_AH, 2);

    if (x->isImm())
    {
        put_opc_reg_imm(GEN_Intel8086_MOV, REG_Intel8086_DL, x->getImm());
    }
    else
    {
        loadTo(REG_Intel8086_DX, x);
        // put_opc_reg_reg(GEN_Intel8086_MOV, REG_Intel8086_DX, x->getReg());
    }

    put_opc_imm(GEN_Intel8086_INT, 0x21);
    put_opc_reg(GEN_Intel8086_POP, REG_Intel8086_DX);
    put_opc_reg(GEN_Intel8086_POP, REG_Intel8086_AX);
}

void Intel8086CodeGenerator::putInt(Item_t x)
{
    load(x);
}

// Convert type of x to type of y.
void Intel8086CodeGenerator::convert(Item_t x, Item_t y)
{
    // TODO: Implement
}

static bool isUsed(unsigned usedRegs, unsigned r)
{
    return ((usedRegs & (0x1 << r)) != 0);
}

unsigned Intel8086CodeGenerator::saveUsedRegs() { return 0; }

void Intel8086CodeGenerator::restoreSavedRegs(unsigned savedRegs)
{
    // TODO: Implement
}

void Intel8086CodeGenerator::setCond(int op, Item_t x, Item_t y)
{
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
            regalloc->freeReg(y->getReg());
            break;

        case GEN_LT:
        case GEN_GE:
            if (x->isImm() && y->isImm())
            {
                x->setImm(x->getImm() < y->getImm() ? 1 : 0);
            }
            else
            {
                unsigned dst;

                if (y->isImm() && TEST_SIMM16_RANGE(y->getImm()))
                {
                    // x->mode != I_CONST
                    load(x);
                    // putF1(SLTI, x->getReg(), x->getReg(),
                    // y->getImm());
                    dst = x->getReg();
                }
                else
                { // x->mode != I_CONST && y->mode != I_CONST
                    load(x);
                    load(y);
                    // putF0(SLT, x->getReg(), x->getReg(),
                    // y->getReg(), 0x0);
                    regalloc->freeReg(y->getReg());
                    dst = x->getReg();
                }
                x->setMode(I_REG);
                x->setReg(dst);
            }
            break;

        case GEN_GT:
        case GEN_LE:
            // NOTE: x and y switches the place
            if (x->isImm() && y->isImm())
            {
                x->setImm(y->getImm() < x->getImm() ? 1 : 0);
            }
            else
            {
                unsigned dst;

                if (x->isImm() && TEST_SIMM16_RANGE(x->getImm()))
                {
                    // y->mode != I_CONST
                    load(y);
                    // putF1(SLTI, y->getReg(), y->getReg(),
                    // x->getImm());
                    dst = y->getReg();
                }
                else
                {
                    load(y);
                    load(x);
                    // putF0(SLT, y->getReg(), y->getReg(),
                    // x->getReg(), 0x0);
                    regalloc->freeReg(x->getReg());
                    dst = y->getReg();
                }
                x->setReg(dst);
                x->setMode(I_REG);
            }
            break;

        default:
            assert(0 && "Code generator: Unknown relation");
    }
}

void Intel8086CodeGenerator::condJump(int op,
                                      Item_t x,
                                      Item_t y,
                                      const char *lab)
{
    load(x);
    load(y);

    put_opc_reg_reg(GEN_Intel8086_CMP, x->getReg(), y->getReg());

    switch (op)
    {
        case GEN_EQ:
            put_opc_lab(GEN_Intel8086_JE, lab);
            break;
        case GEN_NE:
            put_opc_lab(GEN_Intel8086_JNE, lab);
            break;
        case GEN_LT:
            put_opc_lab(GEN_Intel8086_JL, lab);
            break;
        case GEN_GE:
            put_opc_lab(GEN_Intel8086_JGE, lab);
            break;
        case GEN_GT:
            put_opc_lab(GEN_Intel8086_JG, lab);
            break;
        case GEN_LE:
            put_opc_lab(GEN_Intel8086_JLE, lab);
            break;
        default:
            assert(0 && "Code generator: Unknown relation");
    }

    // Release previously reserved registers for x and y
    regalloc->freeReg(x->getReg());
    regalloc->freeReg(y->getReg());
}

void Intel8086CodeGenerator::jump(const char *lab)
{
    put_opc_lab(GEN_Intel8086_JMP, lab);
}

void Intel8086CodeGenerator::addLabel(const char *name,
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

void Intel8086CodeGenerator::addLabel(const char *name,
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

unsigned Intel8086CodeGenerator::getBranchOffset(const char *lab)
{
    LabelMap::iterator it = labels.find(lab);

    if (it != labels.end())
    {
        GenSymbol *tmp = it->second;

        return (tmp->getAdr() - getPC() - 4);
    }
    else
    {
        // TODO: Add fixup
        // addFixup(fixup_, getPC(), lab);
        return -1;
    }
}

unsigned Intel8086CodeGenerator::getJumpTarget(const char *lab)
{
    LabelMap::iterator it = labels.find(lab);

    if (it != labels.end())
    {
        GenSymbol *tmp = it->second;

        return tmp->getAdr();
    }
    else
    {
        // TODO: Add fixup
        // addFixup(fixup_, getPC(), lab);
        return -1;
    }
}

unsigned Intel8086CodeGenerator::getLabelAdr(const char *s)
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
        // addFixup(fixup_, getPC(), s);
        return -1;
    }
}

void Intel8086CodeGenerator::addFixup(unsigned fixupKind,
                                      unsigned fixupAdr,
                                      const char *label)
{
    fixups.push_back(new Fixup(fixupKind, fixupAdr, label));
}

unsigned Intel8086CodeGenerator::fixupToRelocation(unsigned fixupKind)
{
    // TODO: Implement
}

void Intel8086CodeGenerator::resolveFixups() {}

void Intel8086CodeGenerator::emitRData(
    std::vector<std::pair<const char *, const char *> > &rdata)
{
    if (asmOutput && rdata.size() > 0)
    {
        char buf[ASM_STR_SIZE];
        std::pair<const char *, const char *> tmp;

        PUT_ASM("data segment");

        for (unsigned i = 0; i < rdata.size(); i++)
        {
            tmp = rdata[i];
            // PUT_ASM("\t.align 2");
            PUT_ASM_2(buf, "%s db \"%s\"$", tmp.first, tmp.second);
            // PUT_ASM_1(buf, "\t.ascii \"%s\\000\"", tmp.second);
        }
        PUT_ASM("data ends");
    }
}

void Intel8086CodeGenerator::emitLabel(const char *labelName)
{
    if (asmOutput)
    {
        LabelMap::iterator it = labels.find(labelName);

        if (it != labels.end())
        {
            GenSymbol *symbol = it->second;
            char buf[ASM_STR_SIZE];

            if (symbol->getIsFunction())
            {
                /*PUT_ASM("\t.align 2");

                if (symbol->getIsGlobal())
                  PUT_ASM_1(buf, "\t.globl %s", labelName);

                PUT_ASM("\t.set nomips16");
                PUT_ASM("\t.set nomicromips");

                PUT_ASM_1(buf, "\t.ent %s", labelName);
                PUT_ASM_1(buf, "\t.type %s, @function", labelName);*/
                PUT_ASM_1(buf, "%s proc", labelName);
            }
            else
            {
                PUT_ASM_1(buf, "%s:", labelName);
            }
        }
    }
}

void Intel8086CodeGenerator::decode() {}

// Store the code buffer into filename.out file
void Intel8086CodeGenerator::write(const char *output)
{
    int n, totalNumberOfBytes;
    FILE *fp;

    if (asmOutput)
    {
        fp = fopen(output, "w");

        if (fp == NULL)
        {
            printf("error: cannot create output filename \"%s\"\n", output);
            exit(1);
        }

        // data segment
        //   str1 db "12345$"
        //   n dw 0
        //   str2 db "      "
        // ends
        // stack segment
        //   dw 128  dup(0)
        // ends

        fprintf(fp, "data_seg segment\n");
        fprintf(fp, "\tn dw 0\n");
        fprintf(fp, "data_seg ends\n");
        fprintf(fp, "stack_seg segment\n");
        fprintf(fp, "\tdw 128 dup(0)\n");
        fprintf(fp, "stack_seg ends\n");

        fprintf(fp, "code_seg segment\n");

        for (unsigned i = 0; i < asmBuf.size(); i++)
            fprintf(fp, "%s\n", asmBuf[i].c_str());

        // start:
        //   ASSUME ss:stek, cs:code
        //   mov ax, data
        //   mov ds, ax
        //   call main
        // ends
        // end start

        fprintf(fp, "start:\n");
        fprintf(fp, "\tASSUME ds:data_seg, ss:stack_seg, cs:code_seg\n");
        fprintf(fp, "\tcall\tmain\n");
        fprintf(fp, "ends\n");
        fprintf(fp, "end start\n");

        printf("Output: \"%s\"\n", output);
        fclose(fp);
    }
    else
    {
        printf("error: object code generation not supported\n");
        exit(1);
    }
}
