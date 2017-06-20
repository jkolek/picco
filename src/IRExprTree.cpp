// IR expression tree - implementation file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.


#include "../include/IRExprTree.h"
#include "../include/ControlFlowGraph.h"
#include "../include/PrintIRExprVisitor.h"
#include "../include/common.h"

#include <cassert>
#include <vector>

#define REG_SP 0
#define REG_GP 1

static bool
matchBinop(IRExpr *e, unsigned opKind, unsigned lhsKind, unsigned rhsKind)
{
    if (e->getKind() != IR_EK_BINOP)
        return false;

    BinopIRExpr *binop = static_cast<BinopIRExpr *>(e);

    if (opKind != __ANY__ && binop->getBinopKind() != opKind)
        return false;

    if (lhsKind != __ANY__ && binop->getLhs()->getKind() != lhsKind)
        return false;

    if (rhsKind != __ANY__ && binop->getRhs()->getKind() != rhsKind)
        return false;

    return true;
}

// Array reference pattern is:
//   (binop plus
//          __ANY__
//          (binop mul
//                 __ANY__
//                 (const size)))
static bool matchArrayRef(IRExpr *e)
{
    if (e->getKind() != IR_EK_BINOP)
        return false;

    BinopIRExpr *binop = static_cast<BinopIRExpr *>(e);

    if (binop->getBinopKind() != IRBK_PLUS)
        return false;

    // binop->getLhs()->getKind() == __ANY__

    return matchBinop(binop->getRhs(), IRBK_MUL, __ANY__, IR_EK_CONST);
}

// Indirect reference pattern is:
//   (binop plus
//          (const n)
//          (mem ...))
static bool matchIndirectRef(IRExpr *e)
{
    if (e->getKind() != IR_EK_BINOP)
        return false;

    BinopIRExpr *binop = static_cast<BinopIRExpr *>(e);

    if (binop->getBinopKind() != IRBK_PLUS)
        return false;

    if (binop->getLhs()->getKind() != IR_EK_CONST)
        return false;

    if (binop->getRhs()->getKind() != IR_EK_MEM)
        return false;

    return true;
}

// Record reference pattern is:
//   (binop plus
//          (mem ...)
//          (const offset))
static bool matchRecordRef(IRExpr *e)
{
    if (e->getKind() != IR_EK_BINOP)
        return false;

    BinopIRExpr *binop = static_cast<BinopIRExpr *>(e);

    if (binop->getBinopKind() != IRBK_PLUS)
        return false;

    if (binop->getLhs()->getKind() != IR_EK_MEM)
        return false;

    if (binop->getRhs()->getKind() != IR_EK_CONST &&
        binop->getRhs()->getKind() != IR_EK_MEM)
        return false;

    return true;
}

// (matchBinop(expr,
//             IRBK_PLUS,
//             __ANY__,
//             matchBinop(IR_BINOP_RHS(_expr),
//                        IRBK_MUL,
//                        __ANY__,
//                        IR_EK_CONST)))

void IRExprTree::error(const char *msg)
{
    printf("error: %s\n", msg);
    exit(1);
}

void IRExprTree::jumps(IRExpr *expr)
{
    if (!IR_MATCH_SEQ(expr))
        return;

    // FIXME: Create better interface for SeqIRExpr to iterate and delete
    //        elements.

    std::vector<IRExpr *> &elems =
        static_cast<SeqIRExpr *>(expr)->getElements();

    for (std::vector<IRExpr *>::iterator it = elems.begin();
         it != elems.end();
         it++)
    {
        if (IR_MATCH_SEQ((*it)))
        {
            jumps(*it);
        }
        else if (IR_MATCH_FUNCTION((*it)))
        {
            jumps(IR_FUNCTION_BODY((*it)));
        }
        else if (IR_MATCH_JUMP((*it)))
        {
            //  Remove jumps whose target labels are right after them.
            //  i.e.
            //  (jump (name "labx"))
            //  (label "labx")
            //  or
            //  (jump (name "labx"))
            //  (label "laby")
            //  (label "labx")
            //  etc.

            const char *jumpTarget = IR_NAME_VAL(IR_JUMP_TARGET((*it)));

            for (std::vector<IRExpr *>::iterator next = it + 1;
                 next != elems.end();
                 next++)
            {
                if (!IR_MATCH_LABEL((*next)))
                {
                    // Stop when there is no more labels in the sequence
                    break;
                }
                else if (IR_MATCH_LABEL_VAL((*next), jumpTarget))
                {
                    DELETE_IR_EXPR_REF((*it));
                    it = elems.erase(it);
                    break;
                }
            }
        }
        else if (IR_MATCH_CJUMP((*it)))
        {
            //  Remove _conditional jumps whose target labels are right
            //  after them.
            //  i.e.
            //  (cjump (eq (temp "t0")
            //              (temp "t1"))
            //          (name "labx")
            //          (name "laby"))
            //  (label "labx")
            //  (label "laby")

            if (IR_CJUMP_TLAB((*it)) != NULL_IR_EXPR)
            {
                for (std::vector<IRExpr *>::iterator next = it + 1;
                     next != elems.end();
                     next++)
                {
                    if (!IR_MATCH_LABEL((*next)))
                    {
                        // Stop when there is no more labels in the sequence
                        break;
                    }
                    else if (IR_MATCH_LABEL_VAL((*next),
                                                IR_CJUMP_TLAB_NAME((*it))))
                    {
                        IR_CJUMP_SET_TLAB((*it), NULL_IR_EXPR);
                        break;
                    }
                }
            }

            if (IR_CJUMP_FLAB((*it)) != NULL_IR_EXPR)
            {
                for (std::vector<IRExpr *>::iterator next = it + 1;
                     next != elems.end();
                     next++)
                {
                    if (!IR_MATCH_LABEL((*next)))
                    {
                        // Stop when there is no more labels in the sequence
                        break;
                    }
                    else if (IR_MATCH_LABEL_VAL((*next),
                                                IR_CJUMP_FLAB_NAME((*it))))
                    {
                        IR_CJUMP_SET_FLAB((*it), NULL_IR_EXPR);
                        break;
                    }
                }
            }

            if (IR_CJUMP_TLAB((*it)) == NULL_IR_EXPR &&
                IR_CJUMP_FLAB((*it)) == NULL_IR_EXPR)
            {
                DELETE_IR_EXPR_REF((*it));
                it = elems.erase(it);
                continue;
            }
        }
        /*else if (IR_MATCH_CALL(elems[i]))
        {
            // TODO: This should be somewhere else.
            // Set _preserveReturnValue to false if call is out of an
        expression.
            CallIRExpr *callExpr = static_cast<CallIRExpr *>(elems[i]);
            callExpr->setPreserveReturnValue(false);
        }*/
    }
}

void IRExprTree::removeUnusedLabels(IRExpr *seq)
{
    // Used labels
    std::vector<std::string> usedLabs;
    // Pointers to labels
    std::vector<IRExpr *> labs;
    removeUnusedLabelsTraverse(seq, labs, usedLabs);

    // Traverse labs and check if exists in used labs, if not then remove them.
    for (unsigned i = 0; i < labs.size(); i++)
    {
        bool used = false;

        for (unsigned n = 0; n < usedLabs.size(); n++)
        {
            if (strcmp(IR_LABEL_VAL(labs[i]), usedLabs[n].c_str()) == 0)
                used = true;
        }
        if (!used)
        {
            delete labs[i];
            labs[i] = NULL_IR_EXPR;
        }
    }
}

void IRExprTree::removeUnusedLabelsTraverse(
    IRExpr *expr,
    std::vector<IRExpr *> &labs,
    std::vector<std::string> &usedLabs)
{
    if (!IR_MATCH_SEQ(expr))
        return;

    std::vector<IRExpr *> &elems =
        static_cast<SeqIRExpr *>(expr)->getElements();
    unsigned i = 0;

    while (i < elems.size())
    {
        if (IR_MATCH_SEQ(elems[i]))
        {
            removeUnusedLabelsTraverse(elems[i], labs, usedLabs);
        }
        else if (IR_MATCH_JUMP(elems[i]))
        {
            usedLabs.push_back(
                std::string(IR_NAME_VAL(IR_JUMP_TARGET(elems[i]))));
        }
        else if (IR_MATCH_CJUMP(elems[i]))
        {
            CJumpIRExpr *cjump = static_cast<CJumpIRExpr *>(elems[i]);

            if (cjump->getTrueLabel() != NULL_IR_EXPR)
                usedLabs.push_back(
                    std::string(IR_NAME_VAL(cjump->getTrueLabel())));

            if (cjump->getFalseLabel() != NULL_IR_EXPR)
                usedLabs.push_back(
                    std::string(IR_NAME_VAL(cjump->getFalseLabel())));
        }
        else if (IR_MATCH_LABEL(elems[i]))
        {
            labs.push_back(elems[i]);
        }
        i++;
    }
}

// FIXME: Label names are not contained only in JUMPs and CJUMPs, they can be
//        in CONST expressions as well. Elsewhere?
//        CONST expressions usually contain function names, not ordinary
//        labels?
void IRExprTree::replaceLabel(IRExpr *expr, IRExpr *oldlab, IRExpr *newlab)
{
    if (!IR_MATCH_SEQ(expr))
        return;

    std::vector<IRExpr *> &elems =
        static_cast<SeqIRExpr *>(expr)->getElements();
    unsigned i = 0;

    while (i < elems.size())
    {
        if (IR_MATCH_SEQ(elems[i]))
        {
            replaceLabel(elems[i], oldlab, newlab);
        }
        else if (IR_MATCH_FUNCTION(elems[i]))
        {
            replaceLabel(IR_FUNCTION_BODY(elems[i]), oldlab, newlab);
        }
        else if (IR_MATCH_JUMP(elems[i]))
        {
            if (strcmp(IR_LABEL_VAL(oldlab),
                       IR_NAME_VAL(IR_JUMP_TARGET(elems[i]))) == 0)
                IR_JUMP_SET_TARGET(elems[i], IR_LABEL_VAL(newlab));
        }
        else if (IR_MATCH_CJUMP(elems[i]))
        {
            if (IR_CJUMP_TLAB(elems[i]) != NULL_IR_EXPR &&
                strcmp(IR_LABEL_VAL(oldlab), IR_CJUMP_TLAB_NAME(elems[i])) == 0)
                IR_CJUMP_SET_TLAB_S(elems[i], IR_LABEL_VAL(newlab));

            if (IR_CJUMP_FLAB(elems[i]) != NULL_IR_EXPR &&
                strcmp(IR_LABEL_VAL(oldlab), IR_CJUMP_FLAB_NAME(elems[i])) == 0)
                IR_CJUMP_SET_FLAB_S(elems[i], IR_LABEL_VAL(newlab));
        }
        i++;
    }
}

void IRExprTree::removeLabelFollowedByLabel(IRExpr *expr)
{
    if (!IR_MATCH_SEQ(expr))
        return;

    std::vector<IRExpr *> &elems =
        static_cast<SeqIRExpr *>(expr)->getElements();

    for (std::vector<IRExpr *>::iterator it = elems.begin();
         it != elems.end();
         it++)
    {
        if (IR_MATCH_SEQ((*it)))
        {
            removeLabelFollowedByLabel((*it));
        }
        else if (IR_MATCH_FUNCTION((*it)))
        {
            removeLabelFollowedByLabel(IR_FUNCTION_BODY((*it)));
        }
        else if (IR_MATCH_LABEL((*it)))
        {
            // Remove label that if followed by label.
            // i.e.
            //     (label "laby")
            //     (label "labx")
            // etc.
            std::vector<IRExpr *>::iterator next = it + 1;

            while (*next == NULL_IR_EXPR && next != elems.end())
                next++;

            if (next != elems.end() && IR_MATCH_LABEL((*next)))
            {
                // Replace all references to laby with labx
                replaceLabel(_root, (*it), *next);
                DELETE_IR_EXPR_REF((*it));
                it = elems.erase(it);
                continue;
            }
        }
    }
}

void IRExprTree::setPreserveReturnValueToFalse(IRExpr *expr)
{
    if (!IR_MATCH_SEQ(expr))
        return;

    std::vector<IRExpr *> &elems =
        static_cast<SeqIRExpr *>(expr)->getElements();

    for (unsigned i = 0; i < elems.size(); i++)
    {
        if (IR_MATCH_SEQ(elems[i]))
            setPreserveReturnValueToFalse(elems[i]);
        else if (IR_MATCH_FUNCTION(elems[i]))
            setPreserveReturnValueToFalse(IR_FUNCTION_BODY(elems[i]));
        else if (IR_MATCH_CALL(elems[i]))
            static_cast<CallIRExpr *>(elems[i])->setPreserveReturnValue(false);
    }
}

void IRExprTree::controlFlowAnalysis(IRExpr *expr)
{
    assert(expr != NULL_IR_EXPR && expr->getKind() == IR_EK_SEQ);

    std::vector<IRExpr *> &elems =
        static_cast<SeqIRExpr *>(expr)->getElements();
    ControlFlowGraph *cfg = new ControlFlowGraph();

    for (unsigned i = 0; i < elems.size(); i++)
    {
        if (elems[i] != NULL_IR_EXPR)
        {
            if (IR_MATCH_FUNCTION(elems[i]))
                cfg->addFunction(static_cast<FunctionIRExpr *>(elems[i]));
        }
    }

    // if (printCFG)
    //  cfg->print();

    delete cfg;
}

void IRExprTree::emitCode(IRExpr *expr) { expr->traverse(this); }

Item_t ConstIRExpr::traverse(IRExprTree *iret)
{
    CodeGenerator *gen = iret->getCodeGenerator();
    ItemPool *itempool = iret->getItemPool();
    Item_t x;

    if (_labelName != NULL_IR_EXPR)
    {
        x = itempool->createItem(I_REG);
        gen->loadLabelRelative(x, IR_NAME_VAL(_labelName), 0);
    }
    else if (_frameOffset != nullptr)
    {
        FunctionIRExpr *cf = iret->getCurrentFunction();
        InFrame *tmpInFrame = cf->getInFrame(_frameOffset->getValue());
        int realOffset;

        if (tmpInFrame != nullptr)
            realOffset = tmpInFrame->getRealOffset();
        else
            realOffset = _frameOffset->getValue();

        x = itempool->createItem(I_CONST, irtype, realOffset);
    }
    else
    {
        x = itempool->createItem(I_CONST, irtype, _value);
    }

    return x;
}

Item_t NameIRExpr::traverse(IRExprTree *iret)
{
    CodeGenerator *gen = iret->getCodeGenerator();
    ItemPool *itempool = iret->getItemPool();
    Item_t x = itempool->createItem(I_ADR);

    x->setAdr(gen->getJumpTarget(_value)); // FIXME

    return x;
}

Item_t TempIRExpr::traverse(IRExprTree *iret)
{
    CodeGenerator *gen = iret->getCodeGenerator();
    ItemPool *itempool = iret->getItemPool();
    Item_t x = itempool->createItem();

    x->setMode(I_REG);
    x->setReg(gen->getRealReg(_value));
    x->setIRType(irtype);
    if (x->getReg() == REG_SP || x->getReg() == REG_GP)
        x->setIsTemporary(false);
    else
        x->setIsTemporary(true);

    return x;
}

Item_t BinopIRExpr::traverse(IRExprTree *iret)
{
    Item_t x, y;
    CodeGenerator *gen = iret->getCodeGenerator();

    if (_binopKind == IRBK_PLUS)
    {
        ItemPool *itempool = iret->getItemPool();

        if (IR_MATCH_NAME(_lhs) && IR_MATCH_CONST(_rhs))
        {
            //  This is a label + offset:
            //  (plus (name "x")
            //      (const 4))
            x = itempool->createItem(I_REG);
            gen->loadLabelRelative(x, IR_NAME_VAL(_lhs), IR_CONST_VAL(_rhs));
            return x;
        }
        else if (IR_MATCH_TEMP_VAL(_lhs, "GP") && IR_MATCH_CONST(_rhs))
        {
            // This is an address.
            x = itempool->createItem(I_REG);
            gen->loadLabelRelative(x, ".data", IR_CONST_VAL(_rhs));
            return x;
        }
    }
    else if (_binopKind == IRBK_XOR)
    { // Bitwise unary NOT?
        x = nullptr;

        if (IR_MATCH_CONST(_lhs) && IR_CONST_VAL(_lhs) == -1)
            x = _rhs->traverse(iret);
        else if (IR_MATCH_CONST(_rhs) && IR_CONST_VAL(_rhs) == -1)
            x = _lhs->traverse(iret);

        if (x != nullptr)
        {
            gen->unaryOp(GEN_NOT, x);
            return x;
        }
    }

    x = _lhs->traverse(iret);
    y = _rhs->traverse(iret);

    switch (_binopKind)
    {
        case IRBK_PLUS:
            if (x->getIRType() == IR_f32 || x->getIRType() == IR_f64)
            {
                gen->binaryOpFP(GEN_ADD, x, y);
            }
            else
            {
                if (x->isImm() && y->isImm())
                    x->setImm(x->getImm() + y->getImm());
                else if (x->isImm(0))
                    *x = *y;
                else if (y->isImm(0))
                    ; // Do nothing.
                else
                    gen->binaryOp(GEN_ADD, x, y);
            }
            break;
        case IRBK_MINUS:
            if (x->isImm() && y->isImm())
                x->setImm(x->getImm() - y->getImm());
            else if (y->isImm(0))
                ; // Do nothing.
            else
                gen->binaryOp(GEN_SUB, x, y);
            break;
        case IRBK_MUL:
            if (x->isImm() && y->isImm())
                x->setImm(x->getImm() * y->getImm());
            else if (x->isImm(1))
                // Do not multiply with 1, just copy y to x.
                *x = *y;
            else if (y->isImm(1))
                ; // Do nothing.
            else
                gen->binaryOp(GEN_MUL, x, y);
            break;
        case IRBK_DIV:
            if (x->isImm() && y->isImm())
                x->setImm(x->getImm() / y->getImm());
            else if (y->isImm(1))
                ; // Do nothing.
            else
                gen->binaryOp(GEN_DIV, x, y);
            break;
        case IRBK_MOD:
            if (x->isImm() && y->isImm())
                x->setImm(x->getImm() % y->getImm());
            else
                gen->binaryOp(GEN_MOD, x, y);
            break;
        case IRBK_AND:
            if (x->isImm() && y->isImm())
                x->setImm(x->getImm() & y->getImm());
            else
                gen->binaryOp(GEN_AND, x, y);
            break;
        case IRBK_OR:
            if (x->isImm() && y->isImm())
                x->setImm(x->getImm() | y->getImm());
            else
                gen->binaryOp(GEN_OR, x, y);
            break;
        case IRBK_XOR:
            if (x->isImm() && y->isImm())
                x->setImm(x->getImm() ^ y->getImm());
            else
                gen->binaryOp(GEN_XOR, x, y);
            break;
        case IRBK_LSHIFT:
            if (x->isImm() && y->isImm())
                x->setImm(x->getImm() << y->getImm());
            else
                gen->binaryOp(GEN_SHL, x, y);
            break;
        case IRBK_RSHIFT:
            if (x->isImm() && y->isImm())
                x->setImm(x->getImm() >> y->getImm());
            else
                gen->binaryOp(GEN_SHR, x, y);
            break;
        case IRBK_ARSHIFT:
            if (x->isImm() && y->isImm())
                x->setImm(x->getImm() >> y->getImm());
            else
                gen->binaryOp(GEN_SHRA, x, y);
            break;
        default:
            assert("unknown binary operator" && 0);
    }

    x->setIRType(irtype);
    return x;
}

static GenOp getGenOp(IRRelopKind irRelop)
{
    switch (irRelop)
    {
        case IRRK_EQ:
            return GEN_EQ;
        case IRRK_NE:
            return GEN_NE;
        case IRRK_LE:
            return GEN_LE;
        case IRRK_LT:
            return GEN_LT;
        case IRRK_GE:
            return GEN_GE;
        case IRRK_GT:
            return GEN_GT;
    }
    return (GenOp) - 1;
}

static GenOp getGenOpInv(IRRelopKind irRelop)
{
    switch (irRelop)
    {
        case IRRK_EQ:
            return GEN_NE;
        case IRRK_NE:
            return GEN_EQ;
        case IRRK_LE:
            return GEN_GT;
        case IRRK_LT:
            return GEN_GE;
        case IRRK_GE:
            return GEN_LT;
        case IRRK_GT:
            return GEN_LE;
    }
    return (GenOp) - 1;
}

Item_t RelopIRExpr::traverse(IRExprTree *iret)
{
    CodeGenerator *gen = iret->getCodeGenerator();
    Item_t x = _lhs->traverse(iret);
    Item_t y = _rhs->traverse(iret);

    switch (_relopKind)
    {
        case IRRK_EQ:
            gen->setCond(GEN_NE, x, y);
            break;
        case IRRK_NE:
            gen->setCond(GEN_EQ, x, y);
            break;
        case IRRK_LE:
            gen->setCond(GEN_GT, x, y);
            break;
        case IRRK_LT:
            gen->setCond(GEN_GE, x, y);
            break;
        case IRRK_GE:
            gen->setCond(GEN_LT, x, y);
            break;
        case IRRK_GT:
            gen->setCond(GEN_LE, x, y);
            break;
        default:
            assert("unknown relational operator" && 0);
    }

    return x;
}

Item_t MoveIRExpr::traverse(IRExprTree *iret)
{
    CodeGenerator *gen = iret->getCodeGenerator();
    ItemPool *itempool = iret->getItemPool();
    Item_t x = nullptr, y = nullptr;

    // Assignment constraints

    // TODO: Implement type conversion if x and y are of different types.

    if (irtype == IR_f32 || irtype == IR_f64)
    {
        x = _lhs->traverse(iret);
        y = _rhs->traverse(iret);

        gen->storeFP(x, y);

        return x;
    }

    if (IR_MATCH_TEMP_VAL(_lhs, "RV"))
    {
        // Move rhs value to return register.
        x = _rhs->traverse(iret);
        gen->storeReturnValue(x);
    }
    else if (IR_MATCH_TEMP(_lhs) && IR_MATCH_TEMP(_rhs))
    {
        // (move (temp x)
        //         (temp y))
        // =>
        // move rx, ry

        x = itempool->createItem(I_REG);
        x->setIRType(IR_TEMP_TYPE(_lhs));
        x->setReg(gen->getRealReg(IR_TEMP_VAL(_lhs)));

        y = itempool->createItem(I_REG);
        y->setIRType(IR_TEMP_TYPE(_rhs));
        y->setReg(gen->getRealReg(IR_TEMP_VAL(_rhs)));

        gen->move(x, y);
    }
    else if (IR_MATCH_TEMP(_lhs) &&
             matchBinop(_rhs, IRBK_PLUS, IR_EK_TEMP, IR_EK_CONST))
    {
        // Prologue stack adjustment is matched here:
        //
        //     (move (temp "SP")
        //         (binop plus
        //                 (temp "SP")
        //                 (const 8)))
        //     =>
        //     addiu sp, sp, -8

        x = itempool->createItem(I_REG);
        x->setReg(gen->getRealReg(IR_TEMP_VAL(_lhs)));

        y = itempool->createItem(I_REG);
        y->setReg(gen->getRealReg(IR_TEMP_VAL(IR_BINOP_LHS(_rhs))));

        Item_t z = itempool->createItem(I_CONST);
        z->setImm(IR_CONST_VAL(IR_BINOP_RHS(_rhs)));

        gen->binaryOp(GEN_ADD, x, y, z);
    }
    else if (IR_MATCH_TEMP(_lhs) &&
             matchBinop(_rhs, IRBK_MINUS, IR_EK_TEMP, IR_EK_CONST))
    {
        // Epilogue stack adjustment is matched here:
        //
        //     (move (temp "SP")
        //         (binop minus
        //                 (temp "SP")
        //                 (const 8)))
        //     =>
        //     addiu sp, sp, -8

        Item_t z;

        x = itempool->createItem(I_REG);
        x->setReg(gen->getRealReg(IR_TEMP_VAL(_lhs)));

        y = itempool->createItem(I_REG);
        y->setReg(gen->getRealReg(IR_TEMP_VAL(IR_BINOP_LHS(_rhs))));

        z = itempool->createItem(I_CONST);
        z->setImm(IR_CONST_VAL(IR_BINOP_RHS(_rhs)));

        gen->binaryOp(GEN_SUB, x, y, z);
    }
    else if (IR_MATCH_TEMP(_lhs) && IR_MATCH_MEM(_rhs))
    {
        // (move (temp x)
        //         (mem (binop plus
        //                     (temp y)
        //                     (const 8))))
        // =>
        // lw rx, 8(ry)

        x = _lhs->traverse(iret);
        y = _rhs->traverse(iret);

        // Load from memory y to reg x
        gen->loadMem(x, y);
    }
    else
    {
        // (move (mem (binop plus
        //                     (temp y)
        //                     (const 8)))
        //       (temp x))
        // =>
        // sw rx, 8(ry)

        x = _lhs->traverse(iret);
        y = _rhs->traverse(iret);

        gen->store(x, y);
    }

    return x;
}

Item_t MemIRExpr::traverse(IRExprTree *iret)
{
    CodeGenerator *gen = iret->getCodeGenerator();
    ItemPool *itempool = iret->getItemPool();
    Item_t x = itempool->createItem(), y;

    //  Variable pattern example:
    //
    //  (mem (binop plus
    //              (temp SP)
    //              (const 8)))

    if (matchBinop(_expr, IRBK_PLUS, IR_EK_TEMP, IR_EK_CONST))
    {
        if (IR_MATCH_TEMP_VAL(IR_BINOP_LHS(_expr), "GP"))
        {
            x->setMode(I_STATIC);
            x->setAdr(IR_CONST_VAL(IR_BINOP_RHS(_expr)));
            x->setIRType(irtype);
        }
        else
        {
            x->setMode(I_LOCAL);
            x->setReg(gen->getRealReg(IR_TEMP_VAL(IR_BINOP_LHS(_expr))));
            // x->setAdr(IR_CONST_VAL(IR_BINOP_RHS(_expr)));
            y = IR_BINOP_RHS(_expr)->traverse(iret);

            if (y->isImm())
                x->setAdr(y->getImm());

            x->setIRType(irtype);
        }
    }
    else if (matchBinop(_expr, IRBK_PLUS, IR_EK_CONST, IR_EK_TEMP))
    {
        x->setMode(I_LOCAL);
        x->setAdr(IR_CONST_VAL(IR_BINOP_LHS(_expr)));
        x->setReg(gen->getRealReg(IR_TEMP_VAL(IR_BINOP_RHS(_expr))));
        x->setIRType(irtype);
    }
    else if (matchArrayRef(_expr))
    {
        // Array reference
        BinopIRExpr *binop = static_cast<BinopIRExpr *>(_expr);
        BinopIRExpr *rhsBinop = static_cast<BinopIRExpr *>(binop->getRhs());

        x = binop->getLhs()->traverse(iret);
        y = rhsBinop->getLhs()->traverse(iret);
        // TODO: If this line is uncomented the test 'reverse_polish_calc.c'
        // doesn't work.
        x->setIRType(irtype);
        gen->index(x, y);
    }
    else if (matchIndirectRef(_expr))
    {
        BinopIRExpr *binop = static_cast<BinopIRExpr *>(_expr);
        ConstIRExpr *cons = static_cast<ConstIRExpr *>(binop->getLhs());

        x = binop->getRhs()->traverse(iret);
        gen->loadIndRef(x);
        x->setIRType(irtype);
        x->setAdr(cons->getValue());
    }
    else if (matchRecordRef(_expr))
    {
        BinopIRExpr *binop = static_cast<BinopIRExpr *>(_expr);

        x = binop->getLhs()->traverse(iret);
        y = binop->getRhs()->traverse(iret);
        if (y->isImm())
            x->setAdr(x->getAdr() + y->getImm());
        else
            x->setAdr(x->getAdr() + y->getAdr());
        x->setIRType(irtype);
    }

    return x;
}

Item_t CallIRExpr::traverse(IRExprTree *iret)
{
    CodeGenerator *gen = iret->getCodeGenerator();
    ItemPool *itempool = iret->getItemPool();
    Item_t x = nullptr;

    if (IR_MATCH_NAME_VAL(_expr, "putc"))
    {
        x = _args[0]->traverse(iret);
        gen->putChar(x);
    }
    else if (IR_MATCH_NAME_VAL(_expr, "puti"))
    {
        x = _args[0]->traverse(iret);
        gen->putInt(x);
    }
    else
    {
        Item_t y = nullptr;
        // Save used registers.
        // FIXME: If registers $a0-$a3 are holding parameter values they need
        // to be preserved as well.
        unsigned usedRegs = gen->saveUsedRegs();
        int argAdr = 4;

        // Load arguments onto the function stack.
        for (unsigned argNum = 0; argNum < _args.size(); argNum++)
        {
            // FIXME: Change 4 to the size of the corresponding argument.
            // For now we suppose that all arguments are of size 4.
            //y = args[argNum]->traverse(iret);
            //gen->push(y, -argAdr, argNum);
            //argAdr += 4;
            IRExpr *arg = _args[argNum];

            // FIXME: Order of arguments is wrong (test/func_struct_arg.c)!

            for (unsigned argWord = 0; argWord < arg->getSize(); argWord += 4)
            {
                y = arg->traverse(iret);
                y->appendAdr(argWord);
                gen->push(y, -argAdr, argNum);
                argAdr += 4;
            }
        }

        x = _expr->traverse(iret);
        if (IR_MATCH_NAME(_expr))
            gen->call(x, _preserveReturnValue, IR_NAME_VAL(_expr));
        else
            gen->callReg(x, _preserveReturnValue);

        // Restore saved registers.
        gen->restoreSavedRegs(usedRegs);
    }

    return x;
}

Item_t JumpIRExpr::traverse(IRExprTree *iret)
{
    CodeGenerator *gen = iret->getCodeGenerator();

    if (IR_MATCH_NAME(_target))
        gen->jump(IR_NAME_VAL(_target));

    return nullptr;
}

Item_t CJumpIRExpr::traverse(IRExprTree *iret)
{
    CodeGenerator *gen = iret->getCodeGenerator();
    ItemPool *itempool = iret->getItemPool();
    // RelopIRExpr *relop = static_cast<RelopIRExpr *>(_condition);
    // LabelIRExpr *trueLab = static_cast<LabelIRExpr *>(_trueLabel);
    // LabelIRExpr *falseLab = static_cast<LabelIRExpr *>(_falseLabel);
    Item_t x = itempool->createItem(I_REG);
    Item_t y = nullptr;
    IRRelopKind op;

    if (_condition != NULL_IR_EXPR)
        op = static_cast<RelopIRExpr *>(_condition)->getRelopKind();

    if (_trueLabel != NULL_IR_EXPR)
    {
        if (_condition != NULL_IR_EXPR)
        {
            x = static_cast<RelopIRExpr *>(_condition)->getLhs()->traverse(iret);
            y = static_cast<RelopIRExpr *>(_condition)->getRhs()->traverse(iret);
            gen->condJump(getGenOp(op), x, y,
                          static_cast<LabelIRExpr *>(_trueLabel)->getName());
        }
        else
        {
            gen->jump(static_cast<LabelIRExpr *>(_trueLabel)->getName());
        }
    }

    if (_falseLabel != NULL_IR_EXPR)
    {
        if (_condition != NULL_IR_EXPR)
        {
            x = static_cast<RelopIRExpr *>(_condition)->getLhs()->traverse(iret);
            y = static_cast<RelopIRExpr *>(_condition)->getRhs()->traverse(iret);
            gen->condJump(getGenOpInv(op), x, y,
                          static_cast<LabelIRExpr *>(_falseLabel)->getName());
        }
        else
        {
            gen->jump(static_cast<LabelIRExpr *>(_falseLabel)->getName());
        }
    }

    return x;
}

Item_t SeqIRExpr::traverse(IRExprTree *iret)
{
    for (unsigned i = 0; i < _elements.size(); i++)
        _elements[i]->traverse(iret);

    return nullptr;
}

Item_t LabelIRExpr::traverse(IRExprTree *iret)
{
    CodeGenerator *gen = iret->getCodeGenerator();

    gen->addLabel(_name, false, true, _isTemporary);
    if (strcmp(_name, "main") == 0)
        gen->setMainPC();

    if (!_isTemporary)
        gen->emitLabel(_name);

    return nullptr;
}

Item_t FunctionIRExpr::traverse(IRExprTree *iret)
{
    CodeGenerator *gen = iret->getCodeGenerator();
    bool hasCallExpr = false;

    iret->setCurrentFunction(this);

    gen->emitRData(_rdata);

    // Update symbol address.
    gen->addSymbol(_name, gen->getPC());

    gen->addLabel(_name, true /*isFunction*/, true, false /*isTemporary*/);
    if (strcmp(_name, "main") == 0)
        gen->setMainPC();

    gen->emitLabel(_name);

    if (_body == NULL_IR_EXPR)
        return nullptr;

    // Check if there are any function calls.
    IRExprVisitor *visitor = new CountCallIRExprVisitor();

    visitor->visit(static_cast<SeqIRExpr *>(_body));
    hasCallExpr =
        static_cast<CountCallIRExprVisitor *>(visitor)->getCallExprCount() > 0;
    delete visitor;

    // If subroutine has a function call, then we allocate 44 bytes for
    // registers that could be saved during a call:
    //   - 32 bytes for registers $t0 - $t7
    //   - 8 bytes for registers $v0 and $v1
    //   - 4 bytes for the register $ra
    if (hasCallExpr)
        _frameSize += 44;

    // For MIPS architectures stack frame size must be aligned with 8.
    if ((_frameSize % 8) != 0)
        _frameSize += 4;

    // Compute addresses
    for (unsigned n = 0; n < _frame.size(); n++)
    {
        InFrame *tmp = _frame[n];
        unsigned alignedTypeSize = tmp->getTypeSize();

        if (alignedTypeSize % 4)
            alignedTypeSize += (4 - (tmp->getTypeSize() % 4));

        tmp->setRealOffset(_frameSize - tmp->getOffset() - alignedTypeSize);
    }

    // Emit prologue
    gen->emitPrologue(_frameSize, 0, hasCallExpr, nullptr);

    // FIXME: Is this the right place to do this?
    // This is temporary solution, registers should be released when
    // leaving a function.
    // TODO: Check where the registers are reserved but not released. */
    gen->resetUsedRegs();

    _body->traverse(iret);

    // Emit epilogue
    gen->emitEpilogue(_frameSize, 0, hasCallExpr);

    if (gen->getAsmOutput())
        gen->emitAsmEndFunctionDirectives(_name);

    iret->setCurrentFunction(NULL_IR_EXPR);

    return nullptr;
}
