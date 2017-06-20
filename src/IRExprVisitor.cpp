// IR Expr visitor - implementation file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#include "../include/IRExprVisitor.h"
#include "../include/IRExpr.h"

void IRExprVisitor::visit(NullIRExpr *n) {}
void IRExprVisitor::visit(ConstIRExpr *n) {}
void IRExprVisitor::visit(NameIRExpr *n) {}
void IRExprVisitor::visit(TempIRExpr *n) {}

void IRExprVisitor::visit(BinopIRExpr *n)
{
    n->getLhs()->accept(this);
    n->getRhs()->accept(this);
}

void IRExprVisitor::visit(RelopIRExpr *n)
{
    n->getLhs()->accept(this);
    n->getRhs()->accept(this);
}

void IRExprVisitor::visit(MoveIRExpr *n)
{
    n->getLhs()->accept(this);
    n->getRhs()->accept(this);
}

void IRExprVisitor::visit(JumpIRExpr *n) { n->getTarget()->accept(this); }

void IRExprVisitor::visit(CJumpIRExpr *n)
{
    n->getCondition()->accept(this);
    n->getTrueLabel()->accept(this);
    n->getFalseLabel()->accept(this);
}

void IRExprVisitor::visit(MemIRExpr *n) { n->getExpr()->accept(this); }

void IRExprVisitor::visit(CallIRExpr *n) { n->getExpr()->accept(this); }

void IRExprVisitor::visit(SeqIRExpr *n)
{
    std::vector<IRExpr *> elements = n->getElements();

    for (unsigned i = 0; i < elements.size(); i++)
        elements[i]->accept(this);
}

void IRExprVisitor::visit(LabelIRExpr *n) {}

void IRExprVisitor::visit(FunctionIRExpr *n) { n->getBody()->accept(this); }
