// Node classes of IR expression tree.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#include "../include/IRExpr.h"
#include "../include/IRExprVisitor.h"

NullIRExpr *NullIRExpr::_instance = 0;

void NullIRExpr::accept(IRExprVisitor *v) { v->visit(this); }
void ConstIRExpr::accept(IRExprVisitor *v) { v->visit(this); }
void NameIRExpr::accept(IRExprVisitor *v) { v->visit(this); }
void TempIRExpr::accept(IRExprVisitor *v) { v->visit(this); }
void BinopIRExpr::accept(IRExprVisitor *v) { v->visit(this); }
void RelopIRExpr::accept(IRExprVisitor *v) { v->visit(this); }
void MoveIRExpr::accept(IRExprVisitor *v) { v->visit(this); }
void JumpIRExpr::accept(IRExprVisitor *v) { v->visit(this); }
void CJumpIRExpr::accept(IRExprVisitor *v) { v->visit(this); }
void MemIRExpr::accept(IRExprVisitor *v) { v->visit(this); }
void CallIRExpr::accept(IRExprVisitor *v) { v->visit(this); }
void SeqIRExpr::accept(IRExprVisitor *v) { v->visit(this); }
void LabelIRExpr::accept(IRExprVisitor *v) { v->visit(this); }
void FunctionIRExpr::accept(IRExprVisitor *v) { v->visit(this); }
