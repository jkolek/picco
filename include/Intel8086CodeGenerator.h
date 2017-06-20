// Intel8086 code generator - header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef Intel8086_CODEGENERATOR_H
#define Intel8086_CODEGENERATOR_H

#include "CodeGenerator.h"
#include "ELF.h"
#include "ELFObject.h"
#include "SymbolTable.h"
#include "common.h"

enum GenIntel8086Opc
{
    GEN_Intel8086_UNKNOWN,
    GEN_Intel8086_AAA,
    GEN_Intel8086_AAD,
    GEN_Intel8086_AAM,
    GEN_Intel8086_AAS,
    GEN_Intel8086_ADC,
    GEN_Intel8086_ADD,
    GEN_Intel8086_AND,
    GEN_Intel8086_CALL,
    GEN_Intel8086_CMP,
    GEN_Intel8086_DIV,
    GEN_Intel8086_INT,
    GEN_Intel8086_JA,
    GEN_Intel8086_JAE,
    GEN_Intel8086_JB,
    GEN_Intel8086_JBE,
    GEN_Intel8086_JC,
    GEN_Intel8086_JCXZ,
    GEN_Intel8086_JE,
    GEN_Intel8086_JG,
    GEN_Intel8086_JGE,
    GEN_Intel8086_JL,
    GEN_Intel8086_JLE,
    GEN_Intel8086_JMP,
    GEN_Intel8086_JNA,
    GEN_Intel8086_JNAE,
    GEN_Intel8086_JNB,
    GEN_Intel8086_JNBE,
    GEN_Intel8086_JNC,
    GEN_Intel8086_JNE,
    GEN_Intel8086_JNG,
    GEN_Intel8086_JNGE,
    GEN_Intel8086_JNL,
    GEN_Intel8086_JNLE,
    GEN_Intel8086_JNO,
    GEN_Intel8086_JNP,
    GEN_Intel8086_JNS,
    GEN_Intel8086_JNZ,
    GEN_Intel8086_JO,
    GEN_Intel8086_JP,
    GEN_Intel8086_JPE,
    GEN_Intel8086_JPO,
    GEN_Intel8086_JS,
    GEN_Intel8086_JZ,
    GEN_Intel8086_LOOP,
    GEN_Intel8086_LOOPE,
    GEN_Intel8086_LOOPNE,
    GEN_Intel8086_LOOPNZ,
    GEN_Intel8086_LOOPZ,
    GEN_Intel8086_MOV,
    GEN_Intel8086_MOVSB,
    GEN_Intel8086_MOVSW,
    GEN_Intel8086_MUL,
    GEN_Intel8086_NOP,
    GEN_Intel8086_NOT,
    GEN_Intel8086_OR,
    GEN_Intel8086_POP,
    GEN_Intel8086_PUSH,
    GEN_Intel8086_RET,
    GEN_Intel8086_SHL,
    GEN_Intel8086_SHR,
    GEN_Intel8086_SUB,
    GEN_Intel8086_XOR,
};

//
//  Intel 8086 register allocator
//

class Intel8086RegisterAllocator : public RegisterAllocator
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
    bool isReservedFPUReg(unsigned f);
    void reserveFPUReg(unsigned f);
    void freeFPUReg(unsigned f);
    unsigned getUsedRegs();
    void setUsedRegs(unsigned value);

    Intel8086RegisterAllocator();
    ~Intel8086RegisterAllocator();
};

//
//  Intel8086 code generator
//

enum _Intel8086FixupKind
{
    fixup_Intel8086_26,
    fixup_Intel8086_PC16
};

class Intel8086CodeGenerator : public CodeGenerator
{
    void put_opc(GenIntel8086Opc opcode);
    void put_opc_lab(GenIntel8086Opc opcode, const char *lab);
    void put_opc_imm(GenIntel8086Opc opcode, int imm);
    void put_opc_reg(GenIntel8086Opc opcode, unsigned reg);
    void put_opc_reg_reg(GenIntel8086Opc opcode, unsigned reg1, unsigned reg2);
    void put_opc_reg_imm(GenIntel8086Opc opcode, unsigned reg1, int imm);
    void put_opc_reg_mem(GenIntel8086Opc opcode,
                         unsigned reg,
                         unsigned base,
                         int offset);
    void put_opc_mem_reg(GenIntel8086Opc opcode,
                         unsigned base,
                         int offset,
                         unsigned reg);
    void
    put_opc_mem_imm(GenIntel8086Opc opcode, unsigned base, int offset, int imm);

public:
    unsigned getWordSize() { return 4; }
    unsigned getPointerSize() { return 4; }

    void addToDataSection(unsigned adr, unsigned value);

    // unsigned getRealReg(unsigned virtualReg);
    unsigned getRealReg(const char *virtualReg);

    void load(Item_t x);
    void loadTo(unsigned reg, Item_t x);
    void loadMem(Item_t x, Item_t y);
    void loadIndRef(Item_t x);
    void loadLabelRelative(Item_t x, const char *labelName, unsigned offset);

    const char *GenIntel8086OpcToAsm(GenIntel8086Opc opcode);
    unsigned GenIntel8086OpcToObj(GenIntel8086Opc opcode);

    void loadArg(Item_t y);
    void index(Item_t x, Item_t y);
    void field(Item_t x, Object_t obj);
    void store(Item_t x, Item_t y);
    void unaryOp(GenOp op, Item_t x);
    void binaryOp(GenOp op, Item_t x, Item_t y);
    void binaryOp(GenOp op, Item_t x, Item_t y, Item_t z);
    void call(Item_t x, bool preserveReturnValue, const char *fnName);
    void callReg(Item_t x, bool preserveReturnValue);
    void emitPrologue(int frameSize,
                      unsigned paramCount,
                      bool hasCall,
                      SymbolTable *stb);
    void emitEpilogue(int frameSize, unsigned paramCount, bool hasCall);
    void push(Item_t x, unsigned stackRelAddress, unsigned paramNo);
    void moveRegisterToStack(unsigned regNo, unsigned stackRelAddress);
    void pop(void);
    void move(Item_t x, Item_t y);
    void storeReturnValue(Item_t x);
    void emitReturn();
    void emitAsmEndFunctionDirectives(const char *funcName);
    void emitCondInst(GenOp op, Item_t x, Item_t y, Item_t z);
    void label(Item_t x, const char *name);
    void emitGoto(const char *labelName);
    void putChar(Item_t x);
    void putInt(Item_t x);

    // Jumps
    void setCond(int op, Item_t x, Item_t y);
    void condJump(int op, Item_t x, Item_t y, const char *lab);
    void jump(const char *lab);

    // Helper methods

    // Convert type of x to type of y.
    void convert(Item_t x, Item_t y);

    unsigned saveUsedRegs();
    void restoreSavedRegs(unsigned savedRegs);
    void resetUsedRegs() { regalloc->setUsedRegs(0); }
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
    unsigned getLabelAdr(const char *s);
    void addFixup(unsigned fixupKind, unsigned fixupAdr, const char *label);
    unsigned fixupToRelocation(unsigned fixupKind);
    void resolveFixups();

    void emitRData(std::vector<std::pair<const char *, const char *> > &rdata);
    void emitLabel(const char *labelName);

    void decode();
    void defineELF();
    void write(const char *output);

    Intel8086CodeGenerator(ItemPool *Itempool)
    {
        targetMachine = TM_Intel8086;

        asmOutput = false;

        isLittleEndian = false;
        PC = 0;
        objheader.mainPC = -1;
        objheader.readOnlyDataSize = 0;
        rodata_ptr = 0;
        objheader.staticDataSize = 0;
        objheader.symbolsc = 0;
        objheader.relocsc = 0;

        for (unsigned i = 0; i < DATA_NUM; i++)
            data[i] = 0;

        itempool = Itempool;
        regalloc = new Intel8086RegisterAllocator();
    }

    ~Intel8086CodeGenerator()
    {
        delete regalloc;
    }
};

#endif
