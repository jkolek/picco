// IR Expr visitor - header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef IR_EXPR_VISITOR_H
#define IR_EXPR_VISITOR_H

#include "IRExpr.h"

class IdentNode;
class IntegerConstNode;
class CharConstNode;
class NullIRExpr;
class ConstIRExpr;
class TempIRExpr;
class NameIRExpr;
class BinopIRExpr;
class RelopIRExpr;
class MoveIRExpr;
class JumpIRExpr;
class CJumpIRExpr;
class MemIRExpr;
class CallIRExpr;
class SeqIRExpr;
class LabelIRExpr;
class FunctionIRExpr;

class IRExprVisitor
{
public:
    // Virtual destructor
    virtual ~IRExprVisitor() {}

    virtual void visit(NullIRExpr *n);
    virtual void visit(ConstIRExpr *n);
    virtual void visit(NameIRExpr *n);
    virtual void visit(TempIRExpr *n);
    virtual void visit(BinopIRExpr *n);
    virtual void visit(RelopIRExpr *n);
    virtual void visit(MoveIRExpr *n);
    virtual void visit(JumpIRExpr *n);
    virtual void visit(CJumpIRExpr *n);
    virtual void visit(MemIRExpr *n);
    virtual void visit(CallIRExpr *n);
    virtual void visit(SeqIRExpr *n);
    virtual void visit(LabelIRExpr *n);
    virtual void visit(FunctionIRExpr *n);
};

#endif
