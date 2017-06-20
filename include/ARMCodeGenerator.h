// ARM code generator - header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef ARM_CODE_GENERATOR_H
#define ARM_CODE_GENERATOR_H

#include "CodeGenerator.h"
#include "Decoder.h"
#include "ELF.h"
#include "ELFObject.h"
#include "SymbolTable.h"
#include "common.h"

//
//  ARM register allocator
//

class ARMRegisterAllocator : public RegisterAllocator
{
    unsigned usedRegs;
    unsigned nonTmpRegs;
    unsigned usedFPURegs;

public:
    unsigned getReg(void);
    bool isReservedReg(unsigned r);
    void reserveReg(unsigned r);
    void freeReg(unsigned r);
    unsigned getNonTmpReg(void);
    bool isReservedNonTmpReg(unsigned r);
    void reserveNonTmpReg(unsigned r);
    void freeNonTmpReg(unsigned r);
    unsigned getFPUReg(void);
    bool isReservedFPUReg(unsigned f);
    void reserveFPUReg(unsigned f);
    void freeFPUReg(unsigned f);
    unsigned getUsedRegs();

    ARMRegisterAllocator();
    ~ARMRegisterAllocator();
};

//
//  ARM code generator
//

class ARMCodeGenerator : public CodeGenerator
{
    unsigned getRealReg(const char *virtualReg);
    void load(Item_t x);
    void loadFP(Item_t x);

    void compareAndJump(unsigned relop, Item_t x, Item_t y, const char *lab);

public:
    // Writes an instruction encoding to the buffer

    // Data Processing Instructions
    void putDPInst(uint8_t cond,
                   uint8_t opcode,
                   uint8_t I,
                   uint8_t rn,
                   uint8_t rd,
                   uint8_t shiftIsreg,
                   uint8_t shiftType,
                   uint8_t shiftAmountOrRs,
                   unsigned operand2);
    // Single Data Transfer Instructions
    void putSDTInst(uint8_t cond,
                    uint8_t I,
                    uint8_t P,
                    uint8_t U,
                    uint8_t B,
                    uint8_t W,
                    uint8_t L,
                    uint8_t rn,
                    uint8_t rd,
                    unsigned offset);
    // Single Data Transfer Instructions
    void putBDTInst(uint8_t cond,
                    uint8_t P,
                    uint8_t U,
                    uint8_t S,
                    uint8_t W,
                    uint8_t L,
                    uint8_t rn,
                    unsigned regList);
    // Branch and Branch with Link Instruction
    void putBranchInst(uint8_t cond, uint8_t L, unsigned offset);
    // Branch and Exchange Instruction
    void putBXInst(uint8_t cond, uint8_t rn);
    // Multiply and Multiply-Accumulate (MUL, MLA)
    void putMulInst(uint8_t cond,
                    uint8_t A,
                    uint8_t S,
                    uint8_t rd,
                    uint8_t rn,
                    uint8_t rs,
                    uint8_t rm);

    void index(Item_t x, Item_t y);
    void field(Item_t x, Object_t obj);
    void store(Item_t x, Item_t y);
    void storeFP(Item_t x, Item_t y);
    void inc(Item_t x);
    void unaryOp(GenOp op, Item_t x);
    void binaryOp(GenOp op, Item_t x, Item_t y);
    void binaryOp(GenOp op, Item_t x, Item_t y, Item_t z);
    void binaryOpFP(GenOp op, Item_t x, Item_t y);
    void call(Item_t x, bool preserveReturnValue, const char *fnName);
    void callReg(Item_t x, bool preserveReturnValue, const char *fnName);
    void emitPrologue(int frameSize,
                      unsigned paramCount,
                      bool hasCall,
                      SymbolTable *stb);
    void emitEpilogue(int frameSize, unsigned paramCount, bool hasCall);
    void push(Item_t x, unsigned stackRelAddress, unsigned paramNo);
    void moveRegisterToStack(unsigned regNo, unsigned stackRelAddress);
    void pop(void);
    void storeReturnValue(Item_t x);
    void putChar(Item_t x);
    void putInt(Item_t x);

    // Jumps
    void setCond(int op, Item_t x, Item_t y);
    void condJump(int op, Item_t x, Item_t y, const char *lab);
    void jump(const char *lab);

    // Convert type of x to type of y.
    void convert(Item_t x, Item_t y);

    unsigned saveUsedRegs();
    void restoreSavedRegs(unsigned savedRegs);
    unsigned getNonTmpReg() { return regalloc->getNonTmpReg(); }
    void reserveNonTmpReg(unsigned r) { regalloc->reserveNonTmpReg(r); }
    void freeNonTmpReg(unsigned r) { regalloc->freeNonTmpReg(r); }
    unsigned getTmpReg(Item_t x);

    void addLabel(const char *name,
                  bool isFunction,
                  bool isGlobal,
                  bool isTemporary,
                  unsigned adr);
    void addLabel(const char *name,
                  bool isFunction,
                  bool isGlobal,
                  bool isTemporary);
    unsigned getBranchOffset(const char *lab);
    unsigned getJumpTarget(const char *lab);
    unsigned getCallTarget(const char *lab);
    unsigned getLabelAdr(const char *s);
    void addFixup(unsigned fixupKind, unsigned fixupAdr, const char *label);
    unsigned fixupToRelocation(unsigned fixupKind);
    void resolveFixups();

    void emitLabel(const char *labelName);

    void decode();
    void defineELF();
    void write(const char *output);

    ARMCodeGenerator(ItemPool *Itempool)
    {
        targetMachine = TM_ARM;
        isLittleEndian = false;
        PC = 0;
        objheader.mainPC = -1;
        objheader.staticDataSize = 0;
        objheader.symbolsc = 0;
        objheader.relocsc = 0;

        itempool = Itempool;
        regalloc = new ARMRegisterAllocator();
    }

    ~ARMCodeGenerator() { delete regalloc; }
};

#endif
