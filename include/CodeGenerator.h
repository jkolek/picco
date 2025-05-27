// Code generator base classes.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include "Decoder.h"
#include "ELFObject.h"
#include "PiccoObjectFormat.h"
#include "SymbolTable.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>

#define BUFSIZE 4194304 // Buffer size = 4194304 bytes = 4MB
#define ASM_STR_SIZE 256

#define TEST_SIMM12_RANGE(n) n >= -32768 && n <= 32767 // FIXME: Invalid range!
#define TEST_SIMM16_RANGE(n) n >= -32768 && n <= 32767
#define TEST_UIMM16_RANGE(n) n >= 0 && n <= 65535

#define MODE(x) x->mode
#define SIMM(x) x->ival
#define REG(x) x->reg
#define ADR(x) x->adr

#define GLOBAL_POINTER 900
#define STACK_POINTER 901
#define FRAME_POINTER 902
#define LINK_REG 903

#define PUT_ASM(buf) putAsmStr(buf);

#define PUT_ASM_1(inst, buf, prm)                                              \
    {                                                                          \
        snprintf(inst, ASM_STR_SIZE, buf, prm);                                               \
        putAsmStr(inst);                                                       \
    }

#define PUT_ASM_2(inst, buf, prm1, prm2)                                       \
    {                                                                          \
        snprintf(inst, ASM_STR_SIZE, buf, prm1, prm2);                                        \
        putAsmStr(inst);                                                       \
    }

enum TargetMachine
{
    TM_UNKNOWN,
    TM_Intel8086,
    TM_ARM,
};

//
// Item Modes
//

enum ItemMode
{
    I_NONE,

    // Item is an integer constant.
    I_CONST,

    // Item is a floating point constant.
    I_FPIMM,

    // Item is located in stack.
    I_LOCAL,

    // Item is located in static area of memory.
    I_STATIC,
    I_FLD,
    I_PROC,
    I_COND,
    I_STR,
    I_REG,
    I_PAR,
    I_REGIND,
    I_ADR
};

// Generator Operations

// NOTE: Operations starting from EQ to GE must be at positions
// from 0 to 5 because of corresponding invert operations.

enum GenOp
{
    GEN_EQ,
    GEN_NE,
    GEN_LT,
    GEN_LE,
    GEN_GT,
    GEN_GE,
    GEN_ADD,  // Addition
    GEN_SUB,  // Subtraction
    GEN_MUL,  // Multiplication
    GEN_DIV,  // Division
    GEN_MOD,  // Modulo
    GEN_NEG,  // Negation
    GEN_AND,  // Bitwise AND
    GEN_OR,   // Bitwise OR
    GEN_XOR,  // Bitwise XOR
    GEN_NOT,  // Bitwise NOT
    GEN_SHL,  // Left shift
    GEN_SHR,  // Right shift
    GEN_SHRA, // Arithmetical right shift
    GEN_LOAD,
    GEN_STORE
};

enum IRType
{
    IR_i8,
    IR_i16,
    IR_i32,
    IR_i64,
    IR_f32,
    IR_f64
};

class Item
{
    ItemMode _mode;
    GenOp _op;
    int _ival;
    float _fval;
    int _adr;
    IRType _irtype;
    unsigned _reg;     // Assigned register
    bool _isTemporary; // Temporarily register, free up after usage. True is
                       // default.
public:
    Item()
    {
        _isTemporary = true;
        _irtype = IR_i32;
    }

    Item(ItemMode Mode) : _mode(Mode)
    {
        _isTemporary = true;
        _irtype = IR_i32;
    }

    // Copy constructor
    Item(const Item &other)
    {
        _mode = other._mode;
        _op = other._op;
        _ival = other._ival;
        _fval = other._fval;
        _adr = other._adr;
        _irtype = other._irtype;
        _reg = other._reg;
        _isTemporary = other._isTemporary;
    }

    // Copy assignment operator
    Item & operator=(const Item &other)
    {
        if (this != &other)
        {
            _mode = other._mode;
            _op = other._op;
            _ival = other._ival;
            _fval = other._fval;
            _adr = other._adr;
            _irtype = other._irtype;
            _reg = other._reg;
            _isTemporary = other._isTemporary;
        }
        return *this;
    }

    ~Item() {}

    void setSImm32(int ival0) { _ival = ival0; }
    int getSImm32() { return _ival; }

    void setUImm32(uint32_t ival0) { _ival = ival0; }
    uint32_t getUImm32() { return (uint32_t) _ival; }

    void setTmpReg(unsigned reg0)
    {
        _reg = reg0;
        _mode = I_REG;
        _isTemporary = true;
    }

    ItemMode getMode() { return _mode; }

    GenOp getOp() { return _op; }

    int getImm()
    {
        assert(_mode == I_CONST && "Item is not an immediate.");
        return _ival;
    }

    float getFPImm()
    {
        assert(_mode == I_FPIMM && "Item is not a floating point immediate.");
        return _fval;
    }

    int getAdr()
    {
        assert(
            (_mode == I_LOCAL || _mode == I_STATIC || _mode == I_ADR ||
             _mode == I_REGIND) &&
            "Item is not a variable, address, pointer or register indirect.");
        return _adr;
    }

    unsigned getReg()
    {
        assert((_mode == I_REG || _mode == I_REGIND || _mode == I_ADR ||
                _mode == I_LOCAL) &&
               "Item is not a register.");
        return _reg;
    }

    unsigned getBaseReg()
    {
        assert(_mode == I_LOCAL && "Item is not a variable.");
        return _reg;
    }

    void setMode(ItemMode mode0) { _mode = mode0; }

    void setOp(GenOp op0) { _op = op0; }

    void setImm(int ival0) { _ival = ival0; }

    void setFPImm(float fval0) { _fval = fval0; }

    void setAdr(int adr) { _adr = adr; }

    void appendAdr(int value) { _adr += value; }

    void setReg(unsigned reg0) { _reg = reg0; }

    void setIsTemporary(bool value) { _isTemporary = value; }

    // Check if item is a variable, immediate, register, register indirect or
    // a pointer.

    bool isLocal() { return _mode == I_LOCAL; }

    bool isStatic() { return _mode == I_STATIC; }

    bool isImm() { return _mode == I_CONST; }

    bool isImm(int value) { return (_mode == I_CONST && _ival == value); }

    bool isRegister() { return _mode == I_REG; }

    bool getIsTemporary() { return _isTemporary; }

    bool isRegInd() { return _mode == I_REGIND; }

    bool isAddress() { return _mode == I_ADR; }

    void setIRType(IRType irtp) { _irtype = irtp; }

    IRType getIRType() { return _irtype; }
};

typedef Item *Item_t;

class ItemPool
{
    std::vector<Item_t> _itemPool;

public:
    ItemPool() {}
    ~ItemPool()
    {
        for (unsigned i = 0; i < _itemPool.size(); i++)
            delete _itemPool[i];
    }

    Item_t createItem(ItemMode mode)
    {
        _itemPool.push_back(new Item(mode));
        return _itemPool.back();
    }

    Item_t createItem(Item_t y)
    {
        _itemPool.push_back(new Item(*y));
        return _itemPool.back();
    }

    Item_t createItem() { return createItem(I_NONE); }

    Item_t createItem(ItemMode mode, IRType type, int value)
    {
        Item_t x = createItem(mode);
        x->setIRType(type);
        x->setImm(value);
        return x;
    }
};

//
// Register allocator base class
//

class RegisterAllocator
{
public:
    virtual unsigned getReg(void) { return 0; }
    virtual bool isReservedReg(unsigned r) { return false; }
    virtual void reserveReg(unsigned r) {}
    virtual void freeReg(unsigned r) {}

    virtual unsigned getNonTmpReg(void) { return 0; }
    virtual bool isReservedNonTmpReg(unsigned r) { return false; }
    virtual void reserveNonTmpReg(unsigned r) {}
    virtual void freeNonTmpReg(unsigned r) {}

    virtual unsigned getFPUReg(void) { return 0; }
    virtual bool isReservedFPUReg(unsigned f) { return false; }
    virtual void reserveFPUReg(unsigned f) {}
    virtual void freeFPUReg(unsigned f) {}

    virtual unsigned getUsedRegs() { return 0x0; }
    virtual void setUsedRegs(unsigned value) {}
    virtual ~RegisterAllocator() {}
};

class GenSymbol
{
    char name[256];
    unsigned adr;
    bool isFunction;
    bool isGlobal;
    bool isTemporary;
    bool isSection;
    bool isWeak; // Alias
public:
    GenSymbol(const char *Name, unsigned Adr)
    {
        strcpy(name, Name);
        adr = Adr;
        isFunction = false;
        isGlobal = false;
        isTemporary = false;
        isSection = false;
        isWeak = false;
    }
    const char *getName() { return name; }
    unsigned getAdr() { return adr; }
    bool getIsFunction() { return isFunction; }
    bool getIsGlobal() { return isGlobal; }
    bool getIsTemporary() { return isTemporary; }
    bool getIsSection() { return isSection; }

    void setName(const char *value) { strcpy(name, value); }
    void setAdr(unsigned value) { adr = value; }
    void setIsFunction(bool value) { isFunction = value; }
    void setIsGlobal(bool value) { isGlobal = value; }
    void setIsTemporary(bool value) { isTemporary = value; }
    void setIsSection(bool value) { isSection = value; }
};

typedef std::map<const char *, GenSymbol *, CharCompare> LabelMap;

class Fixup
{
public:
    unsigned kind;
    unsigned adr;
    char label[32];

    Fixup(unsigned Kind, unsigned Adr, const char *Lab) : kind(Kind), adr(Adr)
    {
        strcpy(label, Lab);
    }

    unsigned getKind() { return kind; }
    unsigned getAdr() { return adr; }
    const char *getLabel() { return label; }
};

#define RO_DATA_NUM 1000
#define DATA_NUM 1000

//
// Code generator base class
//

class CodeGenerator
{
protected:
    struct objheader_t objheader;
    unsigned char rodata[RO_DATA_NUM]; // TODO: Make this vector
    unsigned rodata_ptr;

    unsigned data[DATA_NUM]; // TODO: Make this vector
    unsigned data_ptr;

    std::vector<struct symbol_t> symbols;
    std::vector<Elf32_Rel> relocs;

    bool asmOutput;

    unsigned PC;
    std::vector<uint8_t> buf; // The code buffer
    bool isLittleEndian;

    std::vector<std::string> asmBuf;

    TargetMachine targetMachine;

    LabelMap labels;
    std::vector<Fixup *> fixups;

    RegisterAllocator *regalloc;
    ItemPool *itempool;
    ELFObject elfObject;

    static const unsigned inverse[6];

    virtual void loadFP(Item_t x) {}
    virtual void
    addRelocation(unsigned type, unsigned offset, const char *value);

    // Order of bits is: 31..0

    void
    setBits(unsigned &binary, unsigned value, unsigned at, unsigned numbits);

    // Code buffer manipulation methods

    void put1b(uint8_t x);
    void put2b(uint16_t x);
    void put4b(uint32_t x);
    void put1b(int pos, uint8_t x);
    void put2b(int pos, uint16_t x);
    void put4b(int pos, uint32_t x);
    uint8_t get1b(int pc);
    uint16_t get2b(int pc);
    uint32_t get4b(int pc);

    // Helper methods

    void setLower16(int pos, uint32_t value);
    uint32_t getLower16(int pos);
    void setLow24(int pos, uint32_t value);
    uint32_t getLow24(int pos);

    // If 2 is a square root of x, returns number of shifts required to reach x,
    // or returns zero otherwise.
    unsigned shiftBy(int x);

    void putAsmStr(const char *str);

public:
    // Word size in bytes
    virtual unsigned getWordSize() { return 4; }
    // Pointer size in bytes
    virtual unsigned getPointerSize() { return 4; }

    virtual void addToDataSection(unsigned adr, unsigned value) {}

    // virtual unsigned getRealReg(unsigned virtualReg) { return 0; }
    virtual unsigned getRealReg(const char *virtualReg) { return 0; }

    virtual void load(Item_t x) {}
    virtual void loadMem(Item_t x, Item_t y) {}
    virtual void loadIndRef(Item_t x) {}
    virtual void
    loadLabelRelative(Item_t x, const char *labelName, unsigned offset)
    {
    }

    TargetMachine getTargetMachine() { return targetMachine; }
    void setTargetMachine(TargetMachine tm) { targetMachine = tm; }

    bool getIsLittleEndian() { return isLittleEndian; }
    void setIsLittleEndian(bool value) { isLittleEndian = value; }

    int getPC() { return PC; }

    virtual bool addSymbol(const char *name, int offset);
    virtual unsigned addStringToRData(const char *value) { return 0; }
    virtual unsigned addStringToReadOnlyData(const char *value);
    virtual void setStaticDataSize(int size);
    virtual void setMainPC();
    virtual void index(Item_t x, Item_t y) {}
    virtual void field(Item_t x, Object_t obj) {}
    virtual void store(Item_t x, Item_t y) {}
    virtual void storeFP(Item_t x, Item_t y) {}
    virtual void unaryOp(GenOp op, Item_t x) {}
    virtual void binaryOp(GenOp op, Item_t x, Item_t y) {}
    virtual void binaryOp(GenOp op, Item_t x, Item_t y, Item_t z) {}
    virtual void binaryOpFP(GenOp op, Item_t x, Item_t y) {}
    virtual void call(Item_t x, bool preserveReturnValue, const char *fnName) {}
    virtual void callReg(Item_t x, bool preserveReturnValue) {}
    virtual void emitPrologue(int frameSize,
                              unsigned paramCount,
                              bool hasCall,
                              SymbolTable *stb)
    {
    }
    virtual void emitEpilogue(int frameSize, unsigned paramCount, bool hasCall)
    {
    }
    virtual void push(Item_t x, unsigned stackRelAddress, unsigned paramNo) {}
    virtual void moveRegisterToStack(unsigned regNo, unsigned stackRelAddress)
    {
    }
    virtual void pop(void) {}
    virtual void move(Item_t x, Item_t y) {}
    virtual void storeReturnValue(Item_t x) {}
    virtual void emitAsmEndFunctionDirectives(const char *funcName) {}
    virtual void emitCondInst(GenOp op, Item_t x, Item_t y, Item_t z) {}
    virtual void putChar(Item_t x) {}
    virtual void putInt(Item_t x) {}
    // Jumps
    virtual void setCond(int op, Item_t x, Item_t y) {}
    virtual void condJump(int op, Item_t x, Item_t y, const char *lab) {}
    virtual void jump(const char *lab) {}

    virtual void convert(Item_t x, Item_t y) {}

    virtual void resetUsedRegs() {}
    virtual unsigned saveUsedRegs() { return 0x0; }
    virtual void restoreSavedRegs(unsigned savedRegs) {}
    virtual unsigned getNonTmpReg() { return 0x0; }
    virtual void reserveNonTmpReg(unsigned r) {}
    virtual void freeNonTmpReg(unsigned r) {}
    virtual unsigned getTmpReg(Item_t x) { return 0; }

    virtual void addLabel(const char *name,
                          bool isFunction,
                          bool isGlobal,
                          bool isTemporary,
                          unsigned adr)
    {
    }
    virtual void
    addLabel(const char *name, bool isFunction, bool isGlobal, bool isTemporary)
    {
    }
    virtual unsigned getBranchOffset(const char *lab) { return 0x0; }
    virtual unsigned getJumpTarget(const char *lab) { return 0x0; }
    virtual unsigned getCallTarget(const char *lab) { return 0x0; }
    virtual unsigned getLabelAdr(const char *s) { return 0x0; }
    virtual void
    addFixup(unsigned fixupKind, unsigned fixupAdr, const char *label)
    {
    }
    virtual void resolveFixups() {}

    virtual void
    emitRData(std::vector<std::pair<const char *, const char *> > &rdata)
    {
    }
    virtual void emitLabel(const char *labelName) {}
    void setAsmOutput(bool value) { asmOutput = value; }
    bool getAsmOutput() { return asmOutput; }

    virtual void decode() {}
    virtual void defineELFHeader() {}
    // FIXME: Move this resolveFixups() somewhere else.
    virtual void write(const char *output);

    virtual ~CodeGenerator()
    {
        for (unsigned i = 0; i < fixups.size(); i++)
            delete fixups[i];

        // Delete GenSymbol labels
        for (LabelMap::iterator it = labels.begin(); it != labels.end(); it++)
            delete it->second;
    }
};

#endif
