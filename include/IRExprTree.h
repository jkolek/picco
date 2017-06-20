// IR expression tree - header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef IR_EXPR_TREE_H
#define IR_EXPR_TREE_H

#include "ARMCodeGenerator.h"
#include "ASTNode.h"
#include "CodeGenerator.h"
#include "CodeGenerator.h"
#include "IRExpr.h"
#include "IRExprTree.h"
#include "IRExprVisitor.h"
#include "PrintIRExprVisitor.h"
#include "Intel8086CodeGenerator.h"

#include <cstdio>

// Pattern matching macros

// TODO: Add the prefix IR_ where missing.

#define IR_MATCH_CONST(e) (e->getKind() == IR_EK_CONST)

#define IR_MATCH_TEMP(e) (e->getKind() == IR_EK_TEMP)

#define IR_MATCH_NAME(e) (e->getKind() == IR_EK_NAME)

#define IR_MATCH_LABEL(e) (e->getKind() == IR_EK_LABEL)

#define IR_MATCH_JUMP(e) (e->getKind() == IR_EK_JUMP)

#define IR_MATCH_CJUMP(e) (e->getKind() == IR_EK_CJUMP)

#define IR_MATCH_CALL(e) (e->getKind() == IR_EK_CALL)

#define IR_MATCH_MEM(e) (e->getKind() == IR_EK_MEM)

#define IR_MATCH_SEQ(e) (e->getKind() == IR_EK_SEQ)

#define IR_MATCH_FUNCTION(e) (e->getKind() == IR_EK_FUNCTION)

#define IR_MATCH_RELOP(e) (e->getKind() == IR_EK_RELOP)

#define MATCH_CONST_VAL(e, c)                                                  \
    ((e->getKind() == IR_EK_CONST) &&                                          \
     (static_cast<ConstIRExpr *>(e)->getValue() == c))

#define IR_MATCH_TEMP_VAL(e, t)                                                \
    ((e->getKind() == IR_EK_TEMP) &&                                           \
     (strcmp(static_cast<TempIRExpr *>(e)->getValue(), t) == 0))

#define IR_MATCH_NAME_VAL(e, n)                                                \
    ((e->getKind() == IR_EK_NAME) &&                                           \
     (strcmp(static_cast<NameIRExpr *>(e)->getValue(), n) == 0))

#define IR_MATCH_LABEL_VAL(e, l)                                               \
    ((e->getKind() == IR_EK_LABEL) &&                                          \
     (strcmp(static_cast<LabelIRExpr *>(e)->getName(), l) == 0))

#define IR_MATCH_BINOP(e, opKind, lhsKind, rhsKind)                            \
    ((e->getKind() == IR_EK_BINOP) &&                                          \
     (static_cast<BinopIRExpr *>(e)->getBinopKind() == opKind) &&              \
     (static_cast<BinopIRExpr *>(e)->getLhs()->getKind() == lhsKind) &&        \
     (static_cast<BinopIRExpr *>(e)->getRhs()->getKind() == rhsKind))

#define IR_CONST_VAL(e) static_cast<ConstIRExpr *>(e)->getValue()
#define IR_TEMP_VAL(e) static_cast<TempIRExpr *>(e)->getValue()
#define IR_NAME_VAL(e) static_cast<NameIRExpr *>(e)->getValue()
#define IR_LABEL_VAL(e) static_cast<LabelIRExpr *>(e)->getName()
#define IR_JUMP_TARGET(e) static_cast<JumpIRExpr *>(e)->getTarget()
#define IR_CJUMP_FLAB(e) static_cast<CJumpIRExpr *>(e)->getFalseLabel()
#define IR_CJUMP_TLAB(e) static_cast<CJumpIRExpr *>(e)->getTrueLabel()
#define IR_BINOP_LHS(e) static_cast<BinopIRExpr *>(e)->getLhs()
#define IR_BINOP_RHS(e) static_cast<BinopIRExpr *>(e)->getRhs()

#define IR_CJUMP_FLAB_NAME(e)                                                  \
    static_cast<NameIRExpr *>(static_cast<CJumpIRExpr *>(e)->getFalseLabel())  \
        ->getValue()
#define IR_CJUMP_TLAB_NAME(e)                                                  \
    static_cast<NameIRExpr *>(static_cast<CJumpIRExpr *>(e)->getTrueLabel())   \
        ->getValue()

#define IR_JUMP_SET_TARGET(e, t) static_cast<JumpIRExpr *>(e)->setTarget(t)
#define IR_CJUMP_SET_FLAB(e, t) static_cast<CJumpIRExpr *>(e)->setFalseLabel(t)
#define IR_CJUMP_SET_TLAB(e, t) static_cast<CJumpIRExpr *>(e)->setTrueLabel(t)

#define IR_CJUMP_SET_FLAB_S(e, t)                                              \
    static_cast<CJumpIRExpr *>(e)->setFalseLabelStr(t)
#define IR_CJUMP_SET_TLAB_S(e, t)                                              \
    static_cast<CJumpIRExpr *>(e)->setTrueLabelStr(t)

#define IR_CONST_TYPE(e) static_cast<ConstIRExpr *>(e)->getIRType()
#define IR_TEMP_TYPE(e) static_cast<TempIRExpr *>(e)->getIRType()

#define IR_FUNCTION_BODY(e) static_cast<FunctionIRExpr *>(e)->getBody()

#define __ANY__ 999

class CodeGeneratorFactory
{
    static TargetMachine strToTargetMachine(const char *target)
    {
        if (strcmp(target, "arm") == 0)
            return TM_ARM;
        else if (strcmp(target, "intel8086") == 0)
            return TM_Intel8086;
        return TM_UNKNOWN;
    }

public:
    static CodeGenerator *createCodeGenerator(const char *target,
                                              ItemPool *itempool)
    {
        CodeGenerator *gen = nullptr;
        TargetMachine tm = strToTargetMachine(target);

        switch (tm)
        {
            case TM_ARM:
                gen = new ARMCodeGenerator(itempool);
                gen->setIsLittleEndian(true);
                break;
            case TM_Intel8086:
                gen = new Intel8086CodeGenerator(itempool);
                break;
        }

        return gen;
    }
};

class LabelIRExpr;
class FunctionIRExpr;

class IRExprTree
{
    IRExpr *_root;

    CodeGenerator *_gen;
    ItemPool _itempool;

    bool _opt;
    unsigned _labelCount;

    IRExpr *_currentFunction;

public:
    IRType stbToIRType(Type_t stbType)
    {
        switch (stbType->kind)
        {
            case T_NONE:
                return IR_i32;
            case T_CHAR:
                return IR_i8;
            case T_SHORT:
                return IR_i16;
            case T_INT:
                return IR_i32;
            case T_UNSIGNED:
                return IR_i32;
            case T_LONG:
                return IR_i64;
            case T_FLOAT:
                return IR_f32;
            case T_DOUBLE:
                return IR_f64;
            case T_VOID:
                return IR_i32;
            case T_ARRAY:
                return IR_i32;
            case T_STRUCT:
                return IR_i32;
            case T_BOOL:
                return IR_i32;
            case T_REAL:
                return IR_i32;
            case T_POINTER:
                return IR_i32;
            case T_ENUM:
                return IR_i32;
            case T_UNION:
                return IR_i32;
            case T_FUNCTION:
                return IR_i32;
        }
        return IR_i32;
    }

    void error(const char *msg);

    void setRoot(IRExpr *Root) { _root = Root; }
    IRExpr *getRoot() { return _root; }

    void setOptimize(bool Opt) { _opt = Opt; }
    bool optimize() { return _opt; }

    void jumps(IRExpr *expr);
    void removeUnusedLabels(IRExpr *expr);
    void removeUnusedLabelsTraverse(IRExpr *expr,
                                    std::vector<IRExpr *> &labs,
                                    std::vector<std::string> &usedLabs);

    void replaceLabel(IRExpr *expr, IRExpr *oldlab, IRExpr *newlab);
    void removeLabelFollowedByLabel(IRExpr *expr);

    // For every call outside of expression set the flag 'preserveReturnValue'
    // to false. Default flag value is true.
    void setPreserveReturnValueToFalse(IRExpr *expr);

    void controlFlowAnalysis(IRExpr *expr);

    // Emit machine code to the buffer.
    void emitCode(IRExpr *expr);

    void printTree();

    void printIRExprTree()
    {
        PrintIRExprVisitor visitor;

        std::cout << std::endl << "IR expression tree:" << std::endl << std::endl;
        visitor.visitIRExpr(_root);
        std::cout << std::endl;
    }


    // If there are no semantic errors this function emits the code and
    // returns 0, otherwise returns the number of semantic errors.
    int emitCode(const char *);

    CodeGenerator *getCodeGenerator() { return _gen; }
    ItemPool *getItemPool() { return &_itempool; }

    unsigned getLabelCount() { return _labelCount++; }

    void setCurrentFunction(IRExpr *CF) { _currentFunction = CF; }
    FunctionIRExpr *getCurrentFunction()
    {
        return static_cast<FunctionIRExpr *>(_currentFunction);
    }

    IRExprTree(char *target)
    {
        _root = nullptr;
        _currentFunction = NULL_IR_EXPR;

        _labelCount = 0;

        _gen = CodeGeneratorFactory::createCodeGenerator(target, &_itempool);
        if (_gen == nullptr)
            error("unknown target machine");
    }

    ~IRExprTree()
    {
        if (_root != nullptr)
            delete _root;

        delete _gen;
    }
};

#endif
