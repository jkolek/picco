// Node classes of IR expression tree.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef IR_EXPR_H
#define IR_EXPR_H

#include "CodeGenerator.h"
#include "IRExprVisitor.h"
#include <vector>

#include <cstdio>
#include <cstring>

// TODO: Create IR canonical form:
//
//    (move (mem w) (plus (mem x) (minus (mem y) (mem z))))
//    =>
//    (move (temp t0) (minus (mem y) (mem z)))
//    (move (temp t1) (plus (mem x) (temp t0)))
//    (move (mem w) (temp t1))

#define ASSIGN_IR_EXPR_REF(x, y)                                               \
    {                                                                          \
        x = y;                                                                 \
        if (x != NULL_IR_EXPR)                                                 \
            x->incRefCount();                                                  \
    }

#define PUSH_BACK_IR_EXPR_REF(v, x)                                            \
    if (x != NULL_IR_EXPR)                                                     \
    {                                                                          \
        v.push_back(x);                                                        \
        x->incRefCount();                                                      \
    }

#define DELETE_IR_EXPR_REF(x)                                                  \
    if (x != NULL_IR_EXPR)                                                     \
    {                                                                          \
        x->decRefCount();                                                      \
        if (x->getRefCount() == 0)                                             \
            delete x;                                                          \
    }

enum IRExprKind
{
    IR_EK_NULL,
    IR_EK_CONST,
    IR_EK_NAME,
    IR_EK_TEMP,
    IR_EK_BINOP,
    IR_EK_RELOP,
    IR_EK_MEM,
    IR_EK_CALL,
    IR_EK_ESEQ,
    IR_EK_MOVE,
    IR_EK_JUMP,
    IR_EK_CJUMP,
    IR_EK_SEQ,
    IR_EK_LABEL,
    IR_EK_FUNCTION
};

class IRExprTree;
class IRExprVisitor;

enum IRBinopKind
{
    IRBK_PLUS,
    IRBK_MINUS,
    IRBK_MUL,
    IRBK_DIV,
    IRBK_MOD,
    IRBK_AND,
    IRBK_OR,
    IRBK_XOR,
    IRBK_LSHIFT,
    IRBK_RSHIFT,
    IRBK_ARSHIFT
};

enum IRRelopKind
{
    IRRK_EQ,
    IRRK_NE,
    IRRK_LT,
    IRRK_GT,
    IRRK_LE,
    IRRK_GE,
    IRRK_ULT,
    IRRK_ULE,
    IRRK_UGT,
    IRRK_UGE
};

#define IR_CONST(t, v) new ConstIRExpr(t, v)
#define IR_NAME(v) new NameIRExpr(v)
#define IR_TEMP(tp, t) new TempIRExpr(tp, t)
#define IR_MOVE(t, x, y) new MoveIRExpr(t, x, y)
#define IR_BINOP(t, op, x, y) new BinopIRExpr(t, op, x, y)
#define IR_RELOP(rel, x, y) new RelopIRExpr(rel, x, y)
#define IR_MEM(t, x) new MemIRExpr(t, x)
#define IR_CALL(e, v) new CallIRExpr(e, v)
//#define CALL(e)        new CallIRExpr(e)
//#define ESEQ
#define IR_JUMP(t) new JumpIRExpr(t)
#define IR_CJUMP(c, t, f) new CJumpIRExpr(c, t, f)
#define IR_SEQ() new SeqIRExpr()
#define IR_LABEL(l) new LabelIRExpr(l)
//#define IR_FUNCTION(n, f, b)   new FunctionIRExpr(n, f, b)
#define IR_FUNCTION(n, f) new FunctionIRExpr(n, f)

class FrameOffset
{
    int _value;

public:
    FrameOffset(int Value) : _value(Value) {}
    int getValue() { return _value; }
};

class IRExpr
{
protected:
    IRExprKind kind;
    Type_t type;
    IRType irtype;
    unsigned size;
    unsigned refCount;
    bool isBBLeader;

public:
    IRExprKind getKind() { return kind; }

    IRExpr()
    {
        refCount = 0;
        isBBLeader = false;
    }

    IRExpr(IRExprKind Kind)
    {
        kind = Kind;
        refCount = 0;
        isBBLeader = false;
    }

    // Virtual destructor
    virtual ~IRExpr() {}

    Type_t getType() { return type; }
    void setType(Type_t t) { type = t; }

    IRType getIRType() { return irtype; }
    void setIRType(IRType t) { irtype = t; }

    unsigned getSize() { return size; }
    void setSize(unsigned value) { size = value; }

    // Virtual functions
    virtual Item_t traverse(IRExprTree *iret) { return nullptr; }
    virtual void accept(IRExprVisitor *v) {}

    void incRefCount() { refCount++; }
    void decRefCount() { refCount--; }
    unsigned getRefCount() { return refCount; }

    bool getIsBBLeader() { return isBBLeader; }
    void setIsBBLeader(bool value) { isBBLeader = value; }
};

class NullIRExpr : public IRExpr
{
    static NullIRExpr *_instance;

public:
    NullIRExpr() { kind = IR_EK_NULL; }

    ~NullIRExpr() {}

    void accept(IRExprVisitor *v);

    static NullIRExpr *getInstance()
    {
        if (!_instance)
            _instance = new NullIRExpr();
        return _instance;
    }
};

// TODO: Use NULL_IR_EXPR instead of NULL for IRExpr* nodes.
#define NULL_IR_EXPR NullIRExpr::getInstance()

class NameIRExpr : public IRExpr
{
    char _value[MAXSTR];

public:
    NameIRExpr(const char *VALUE)
    {
        strcpy(_value, VALUE);
        irtype = IR_i32;
        kind = IR_EK_NAME;
    }

    const char *getValue() { return _value; }
    void setValue(const char *Value) { strcpy(_value, Value); }

    Item_t traverse(IRExprTree *iret);
    void accept(IRExprVisitor *v);
};

class ConstIRExpr : public IRExpr
{
    int _value;
    IRExpr *_labelName;
    FrameOffset *_frameOffset;

public:
    ConstIRExpr(IRType t, int Value) : _value(Value)
    {
        irtype = t;
        kind = IR_EK_CONST;
        _labelName = NULL_IR_EXPR;
        _frameOffset = nullptr;
    }

    ConstIRExpr(IRType t, NameIRExpr *LabelName)
    {
        irtype = t;
        kind = IR_EK_CONST;
        _frameOffset = nullptr;
        _value = 0;

        ASSIGN_IR_EXPR_REF(_labelName, LabelName);
    }

    ConstIRExpr(IRType t, FrameOffset *offset) : _frameOffset(offset)
    {
        irtype = t;
        kind = IR_EK_CONST;
        _labelName = NULL_IR_EXPR;
        _value = 0;
    }

    ~ConstIRExpr()
    {
        DELETE_IR_EXPR_REF(_labelName);
        if (_frameOffset)
            delete _frameOffset;
    }

    int getValue() { return _value; }
    NameIRExpr *getLabelName() { return static_cast<NameIRExpr *>(_labelName); }

    Item_t traverse(IRExprTree *iret);
    void accept(IRExprVisitor *v);
};

class TempIRExpr : public IRExpr
{
    char _value[MAXSTR];

public:
    TempIRExpr(const char *VALUE)
    {
        irtype = IR_i32;
        strcpy(_value, VALUE);
        kind = IR_EK_TEMP;
    }

    TempIRExpr(IRType irtp, const char *VALUE)
    {
        irtype = irtp;
        strcpy(_value, VALUE);
        kind = IR_EK_TEMP;
    }

    const char *getValue() { return _value; }

    Item_t traverse(IRExprTree *iret);
    void accept(IRExprVisitor *v);
};

class BinopIRExpr : public IRExpr
{
    IRBinopKind _binopKind;
    IRExpr *_lhs;
    IRExpr *_rhs;

public:
    BinopIRExpr(IRBinopKind bk, IRExpr *Lhs, IRExpr *Rhs) : _binopKind(bk)
    {
        irtype = IR_i32;
        kind = IR_EK_BINOP;

        ASSIGN_IR_EXPR_REF(_lhs, Lhs);
        ASSIGN_IR_EXPR_REF(_rhs, Rhs);
    }

    BinopIRExpr(IRType t, IRBinopKind bk, IRExpr *Lhs, IRExpr *Rhs)
        : _binopKind(bk)
    {
        irtype = t;
        kind = IR_EK_BINOP;

        ASSIGN_IR_EXPR_REF(_lhs, Lhs);
        ASSIGN_IR_EXPR_REF(_rhs, Rhs);
    }

    ~BinopIRExpr()
    {
        // FIXME: This causes segmentation fault.
        // Look at PostdecrementExprASTNode::emitIR() located in
        // AbstractSyntaxTree.cpp.

        DELETE_IR_EXPR_REF(_lhs);
        DELETE_IR_EXPR_REF(_rhs);
    }

    IRExpr *getLhs() { return _lhs; }
    IRExpr *getRhs() { return _rhs; }

    IRBinopKind getBinopKind() { return _binopKind; }
    Item_t traverse(IRExprTree *iret);
    void accept(IRExprVisitor *v);
};

class RelopIRExpr : public IRExpr
{
    IRRelopKind _relopKind;
    IRExpr *_lhs;
    IRExpr *_rhs;

public:
    RelopIRExpr(IRRelopKind rk, IRExpr *Lhs, IRExpr *Rhs) : _relopKind(rk)
    {
        irtype = IR_i32;
        kind = IR_EK_RELOP;

        ASSIGN_IR_EXPR_REF(_lhs, Lhs);
        ASSIGN_IR_EXPR_REF(_rhs, Rhs);
    }

    RelopIRExpr(IRType t, IRRelopKind rk, IRExpr *Lhs, IRExpr *Rhs)
        : _relopKind(rk)
    {
        irtype = t;
        kind = IR_EK_RELOP;

        ASSIGN_IR_EXPR_REF(_lhs, Lhs);
        ASSIGN_IR_EXPR_REF(_rhs, Rhs);
    }

    ~RelopIRExpr()
    {
        DELETE_IR_EXPR_REF(_lhs);
        DELETE_IR_EXPR_REF(_rhs);
    }

    IRExpr *getLhs() { return _lhs; }
    IRExpr *getRhs() { return _rhs; }

    IRRelopKind getRelopKind() { return _relopKind; }
    Item_t traverse(IRExprTree *iret);
    void accept(IRExprVisitor *v);
};

class MoveIRExpr : public IRExpr
{
    IRExpr *_lhs;
    IRExpr *_rhs;

public:
    MoveIRExpr(IRExpr *Lhs, IRExpr *Rhs)
    {
        kind = IR_EK_MOVE;
        irtype = IR_i32;

        ASSIGN_IR_EXPR_REF(_lhs, Lhs);
        ASSIGN_IR_EXPR_REF(_rhs, Rhs);
    }

    MoveIRExpr(IRType Irtype, IRExpr *Lhs, IRExpr *Rhs)
    {
        kind = IR_EK_MOVE;
        irtype = Irtype;

        ASSIGN_IR_EXPR_REF(_lhs, Lhs);
        ASSIGN_IR_EXPR_REF(_rhs, Rhs);
    }

    ~MoveIRExpr()
    {
        DELETE_IR_EXPR_REF(_lhs);
        DELETE_IR_EXPR_REF(_rhs);
    }

    IRExpr *getLhs() { return _lhs; }
    IRExpr *getRhs() { return _rhs; }

    Item_t traverse(IRExprTree *iret);
    void accept(IRExprVisitor *v);
};

class JumpIRExpr : public IRExpr
{
    IRExpr *_target;

public:
    JumpIRExpr(IRExpr *Target)
    {
        kind = IR_EK_JUMP;
        irtype = IR_i32;

        ASSIGN_IR_EXPR_REF(_target, Target);
    }

    ~JumpIRExpr() { DELETE_IR_EXPR_REF(_target); }

    IRExpr *getTarget() { return _target; }
    void setTarget(const char *Target)
    {
        if (_target->getKind() == IR_EK_NAME)
        {
            NameIRExpr *tmp = static_cast<NameIRExpr *>(_target);
            tmp->setValue(Target);
        }
    }

    Item_t traverse(IRExprTree *iret);
    void accept(IRExprVisitor *v);
};

class CJumpIRExpr : public IRExpr
{
    IRExpr *_condition;
    IRExpr *_trueLabel;
    IRExpr *_falseLabel;

public:
    CJumpIRExpr(IRExpr *Condition, IRExpr *TrueLabel, IRExpr *FalseLabel)
    {
        irtype = IR_i32;
        kind = IR_EK_CJUMP;

        ASSIGN_IR_EXPR_REF(_condition, Condition);
        ASSIGN_IR_EXPR_REF(_trueLabel, TrueLabel);
        ASSIGN_IR_EXPR_REF(_falseLabel, FalseLabel);
    }

    ~CJumpIRExpr()
    {
        DELETE_IR_EXPR_REF(_condition);
        DELETE_IR_EXPR_REF(_falseLabel);
        DELETE_IR_EXPR_REF(_trueLabel);
    }

    IRExpr *getCondition() { return _condition; }
    IRExpr *getTrueLabel() { return _trueLabel; }
    IRExpr *getFalseLabel() { return _falseLabel; }

    void setTrueLabel(IRExpr *val)
    {
        if (val == NULL_IR_EXPR)
        {
            DELETE_IR_EXPR_REF(_trueLabel);
            _trueLabel = NULL_IR_EXPR;
        }
        else
        {
            ASSIGN_IR_EXPR_REF(_trueLabel, val);
        }
    }

    void setFalseLabel(IRExpr *val)
    {
        if (val == NULL_IR_EXPR)
        {
            DELETE_IR_EXPR_REF(_falseLabel);
            _falseLabel = NULL_IR_EXPR;
        }
        else
        {
            ASSIGN_IR_EXPR_REF(_falseLabel, val);
        }
    }

    void setTrueLabelStr(const char *Target)
    {
        if (_trueLabel->getKind() == IR_EK_NAME)
        {
            NameIRExpr *tmp = static_cast<NameIRExpr *>(_trueLabel);
            tmp->setValue(Target);
        }
    }

    void setFalseLabelStr(const char *Target)
    {
        if (_falseLabel->getKind() == IR_EK_NAME)
        {
            NameIRExpr *tmp = static_cast<NameIRExpr *>(_falseLabel);
            tmp->setValue(Target);
        }
    }

    Item_t traverse(IRExprTree *iret);
    void accept(IRExprVisitor *v);
};

class MemIRExpr : public IRExpr
{
    IRExpr *_expr;

public:
    MemIRExpr(IRType t, IRExpr *e)
    {
        irtype = t;
        kind = IR_EK_MEM;

        ASSIGN_IR_EXPR_REF(_expr, e);
    }

    ~MemIRExpr() { DELETE_IR_EXPR_REF(_expr); }

    IRExpr *getExpr() { return _expr; }

    Item_t traverse(IRExprTree *iret);
    void accept(IRExprVisitor *v);
};

class CallIRExpr : public IRExpr
{
    IRExpr *_expr;
    std::vector<IRExpr *> _args;
    bool _preserveReturnValue;

public:
    CallIRExpr(IRExpr *e, std::vector<IRExpr *> &Args)
    {
        irtype = IR_i32;
        kind = IR_EK_CALL;

        ASSIGN_IR_EXPR_REF(_expr, e);

        for (unsigned i = 0; i < Args.size(); i++)
            PUSH_BACK_IR_EXPR_REF(_args, Args[i]);

        _preserveReturnValue = true;
    }

    CallIRExpr(IRExpr *e)
    {
        irtype = IR_i32;
        kind = IR_EK_CALL;
        _preserveReturnValue = true;

        ASSIGN_IR_EXPR_REF(_expr, e);
    }

    ~CallIRExpr()
    {
        DELETE_IR_EXPR_REF(_expr);
        for (unsigned i = 0; i < _args.size(); i++)
            DELETE_IR_EXPR_REF(_args[i])
    }

    bool getPreserveReturnValue() { return _preserveReturnValue; }
    void setPreserveReturnValue(bool val) { _preserveReturnValue = val; }

    IRExpr *getExpr() { return _expr; }
    std::vector<IRExpr *> &getArgs() { return _args; }

    Item_t traverse(IRExprTree *iret);
    void accept(IRExprVisitor *v);
};

class SeqIRExpr : public IRExpr
{
    std::vector<IRExpr *> _elements;

public:
    SeqIRExpr()
    {
        kind = IR_EK_SEQ;
        irtype = IR_i32;
    }

    SeqIRExpr(IRExpr *n)
    {
        kind = IR_EK_SEQ;
        irtype = IR_i32;
        add(n);
    }

    ~SeqIRExpr()
    {
        for (unsigned i = 0; i < _elements.size(); i++)
            DELETE_IR_EXPR_REF(_elements[i]);
    }

    std::vector<IRExpr *> &getElements() { return _elements; }
    void setElements(std::vector<IRExpr *> e) { _elements = e; }

    void add(IRExpr *n)
    {
        // If n is a tree list, append its elements to these elements
        if (n->getKind() == IR_EK_SEQ)
        {
            std::vector<IRExpr *> &tmpVec =
                static_cast<SeqIRExpr *>(n)->getElements();

            for (unsigned i = 0; i < tmpVec.size(); i++)
                PUSH_BACK_IR_EXPR_REF(_elements, tmpVec[i]);
            // We don't need n anymore
            delete n;
        }
        else
        {
            PUSH_BACK_IR_EXPR_REF(_elements, n);
        }
    }

    void addElement(IRExpr *e) { PUSH_BACK_IR_EXPR_REF(_elements, e); }
    int size() { return _elements.size(); }

    Item_t traverse(IRExprTree *ast);
    void accept(IRExprVisitor *v);
};

class LabelIRExpr : public IRExpr
{
    char _name[MAXSTR];
    bool _isTemporary;

public:
    LabelIRExpr(const char *Name)
    {
        kind = IR_EK_LABEL;
        irtype = IR_i32;
        strcpy(_name, Name);
        _isTemporary = false;
    }

    char *getName() { return _name; }
    bool getIsTemporary() { return _isTemporary; }
    void setIsTemporary(bool value) { _isTemporary = value; }

    Item_t traverse(IRExprTree *iret);
    void accept(IRExprVisitor *v);
};

class Access
{
};

class InFrame : public Access
{
    int _offset;
    int _realOffset; // Real machine offset
    int _typeSize;

public:
    InFrame(int Offset, int TypeSize) : _offset(Offset), _typeSize(TypeSize) {}
    int getOffset() { return _offset; }
    void setOffset(int value) { _offset = value; }
    int getTypeSize() { return _typeSize; }
    int getRealOffset() { return _realOffset; }
    void setRealOffset(int value) { _realOffset = value; }
};

class InReg : public Access
{
    TempIRExpr *_temp;

public:
    InReg(TempIRExpr *t) : _temp(t) {}
    TempIRExpr *getTemp() { return _temp; }
};

/*class FormalEscape
{
public:
    bool value;
};

class Frame
{
    LabelIRExpr *name;
    std::vector<FormalEscape *>formals;
public:
    Frame(LabelIRExpr *Name, std::vector<FormalEscape *>&Formals)
        : name(Name), formals(Formals)
    {
        //
    }
    virtual Access *allocLocal(bool escape) {}
};*/

class FunctionIRExpr : public IRExpr
{
    char _name[MAXSTR];
    unsigned _frameSize;
    IRExpr *_body;
    std::vector<InFrame *> _frame;
    std::vector<InReg *> _regs;
    std::vector<std::pair<const char *, const char *> > _rdata;

public:
    FunctionIRExpr(const char *Name, unsigned FrameSize, SeqIRExpr *Body)
        : _frameSize(FrameSize)
    {
        kind = IR_EK_FUNCTION;
        irtype = IR_i32;
        strcpy(_name, Name);
        ASSIGN_IR_EXPR_REF(_body, Body);
    }

    FunctionIRExpr(const char *Name, unsigned FrameSize) : _frameSize(FrameSize)
    {
        kind = IR_EK_FUNCTION;
        irtype = IR_i32;
        strcpy(_name, Name);
        _body = nullptr;
    }

    ~FunctionIRExpr()
    {
        DELETE_IR_EXPR_REF(_body);
        for (unsigned n = 0; n < _frame.size(); n++)
            delete _frame[n];
        for (unsigned n = 0; n < _regs.size(); n++)
            delete _regs[n];
        // FIXME: Deallocate rdata elements.
        /*for (unsigned n = 0; n < rdata.size(); n++)
        {
        free(rdata[n].first);
        free(rdata[n].second);
        }*/
    }

    char *getName() { return _name; }
    unsigned getFrameSize() { return _frameSize; }
    void setFrameSize(unsigned value) { _frameSize = value; }
    SeqIRExpr *getBody() { return static_cast<SeqIRExpr *>(_body); }

    void setBody(SeqIRExpr *Body) { ASSIGN_IR_EXPR_REF(_body, Body); }

    void addInFrame(int offset, int typeSize)
    {
        _frame.push_back(new InFrame(offset, typeSize));
    }

    void addInReg(TempIRExpr *t) { _regs.push_back(new InReg(t)); }

    void addStringConst(const char *label, const char *value)
    {
        char *Label = (char *)malloc(256);
        char *Value = (char *)malloc(256);
        strcpy(Label, label);
        strcpy(Value, value);
        _rdata.push_back(std::make_pair(Label, Value));
    }

    InFrame *getInFrame(unsigned offset)
    {
        for (unsigned n = 0; n < _frame.size(); n++)
        {
            InFrame *tmp = _frame[n];
            if (tmp->getOffset() == offset)
                return _frame[n];
        }
        return nullptr;
    }

    Item_t traverse(IRExprTree *iret);
    void accept(IRExprVisitor *v);
};

#endif
