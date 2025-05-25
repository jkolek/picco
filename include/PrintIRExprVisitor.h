// Print IR Expr visitor - header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef PRINT_IR_EXPR_VISITOR_H
#define PRINT_IR_EXPR_VISITOR_H

#include "IRExpr.h"
#include "IRExprVisitor.h"

#include <iostream>

class PrintIRExprVisitor : public IRExprVisitor
{

    int _level;

#define TAB_SIZE 4

    void printTab(int n)
    {
        int i, x;

        x = n * TAB_SIZE;
        for (i = 0; i < x; i++)
            std::cout << " ";
    }

    const char *IRTypeToStr(IRType irtp)
    {
        switch (irtp)
        {
            case IR_i8:
                return "i8";
            case IR_i16:
                return "i16";
            case IR_i32:
                return "i32";
            default:
                return "unknown IR type";
        }
    }

public:
    PrintIRExprVisitor() { _level = 0; }

    void visitIRExpr(IRExpr *e)
    {
        switch (e->getKind())
        {
            case IR_EK_CONST:
                visit(static_cast<ConstIRExpr *>(e));
                break;
            case IR_EK_NAME:
                visit(static_cast<NameIRExpr *>(e));
                break;
            case IR_EK_TEMP:
                visit(static_cast<TempIRExpr *>(e));
                break;
            case IR_EK_BINOP:
                visit(static_cast<BinopIRExpr *>(e));
                break;
            case IR_EK_RELOP:
                visit(static_cast<RelopIRExpr *>(e));
                break;
            case IR_EK_MEM:
                visit(static_cast<MemIRExpr *>(e));
                break;
            case IR_EK_CALL:
                visit(static_cast<CallIRExpr *>(e));
                break;
            // case IR_EK_ESEQ:
            case IR_EK_MOVE:
                visit(static_cast<MoveIRExpr *>(e));
                break;
            case IR_EK_JUMP:
                visit(static_cast<JumpIRExpr *>(e));
                break;
            case IR_EK_CJUMP:
                visit(static_cast<CJumpIRExpr *>(e));
                break;
            case IR_EK_SEQ:
                visit(static_cast<SeqIRExpr *>(e));
                break;
            case IR_EK_LABEL:
                visit(static_cast<LabelIRExpr *>(e));
                break;
            case IR_EK_FUNCTION:
                visit(static_cast<FunctionIRExpr *>(e));
                break;
            default:
                assert("unknown ir expression kind" && 0);
        }
    }

    void visit(ConstIRExpr *n)
    {
        printTab(_level + 1);
        if (n->getLabelName() != nullptr)
        {
            std::cout << "(const " << IRTypeToStr(n->getIRType()) << " ";
            n->getLabelName()->accept(this);
            std::cout << ")" << std::endl;
        }
        else
        {
            std::cout << "(const " << IRTypeToStr(n->getIRType()) << " "
                      << n->getValue() << ")" << std::endl;
        }
    }

    void visit(NameIRExpr *n)
    {
        printTab(_level + 1);
        std::cout << "(name " << n->getValue() << ")" << std::endl;
    }

    void visit(TempIRExpr *n)
    {
        printTab(_level + 1);
        std::cout << "(temp " << n->getValue() << ")" << std::endl;
    }

    void visit(BinopIRExpr *n)
    {
        _level++;

        printTab(_level);
        switch (n->getBinopKind())
        {
            case IRBK_PLUS:
                std::cout << "(binop plus " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRBK_MINUS:
                std::cout << "(binop minus " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRBK_MUL:
                std::cout << "(binop mul " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRBK_DIV:
                std::cout << "(binop div " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRBK_MOD:
                std::cout << "(binop mod " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRBK_AND:
                std::cout << "(binop and " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRBK_OR:
                std::cout << "(binop or " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRBK_XOR:
                std::cout << "(binop xor " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRBK_LSHIFT:
                std::cout << "(binop lshift " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRBK_RSHIFT:
                std::cout << "(binop rshift " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRBK_ARSHIFT:
                std::cout << "(binop arshift " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
        }

        n->getLhs()->accept(this);

        n->getRhs()->accept(this);

        printTab(_level);
        std::cout << ")" << std::endl;

        _level--;
    }

    void visit(RelopIRExpr *n)
    {
        _level++;

        printTab(_level);
        switch (n->getRelopKind())
        {
            case IRRK_EQ:
                std::cout << "(relop eq " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRRK_NE:
                std::cout << "(relop ne " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRRK_LT:
                std::cout << "(relop lt " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRRK_GT:
                std::cout << "(relop gt " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRRK_LE:
                std::cout << "(relop le " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRRK_GE:
                std::cout << "(relop ge " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRRK_ULT:
                std::cout << "(relop ult " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRRK_ULE:
                std::cout << "(relop ule " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRRK_UGT:
                std::cout << "(relop ugt " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
            case IRRK_UGE:
                std::cout << "(relop uge " << IRTypeToStr(n->getIRType())
                          << std::endl;
                break;
        }

        n->getLhs()->accept(this);

        n->getRhs()->accept(this);

        printTab(_level);
        std::cout << ")" << std::endl;

        _level--;
    }

    void visit(MoveIRExpr *n)
    {
        _level++;

        printTab(_level);
        std::cout << "(move " << IRTypeToStr(n->getIRType()) << std::endl;

        n->getLhs()->accept(this);

        n->getRhs()->accept(this);

        printTab(_level);
        std::cout << ")" << std::endl;

        _level--;
    }

    void visit(JumpIRExpr *n)
    {
        _level++;

        printTab(_level);
        std::cout << "(jump" << std::endl;

        n->getTarget()->accept(this);

        printTab(_level);
        std::cout << ")" << std::endl;

        _level--;
    }

    void visit(CJumpIRExpr *n)
    {
        _level++;

        printTab(_level);
        std::cout << "(cjump" << std::endl;

        n->getCondition()->accept(this);

        printTab(_level + 1);
        std::cout << "true label:" << std::endl;
        if (n->getTrueLabel() != NULL_IR_EXPR)
            n->getTrueLabel()->accept(this);

        printTab(_level + 1);
        std::cout << "false label:" << std::endl;
        if (n->getFalseLabel() != NULL_IR_EXPR)
            n->getFalseLabel()->accept(this);

        printTab(_level);
        std::cout << ")" << std::endl;

        _level--;
    }

    void visit(MemIRExpr *n)
    {
        _level++;

        printTab(_level);
        std::cout << "(mem " << IRTypeToStr(n->getIRType()) << std::endl;

        n->getExpr()->accept(this);

        printTab(_level);
        std::cout << ")" << std::endl;

        _level--;
    }

    void visit(CallIRExpr *n)
    {
        _level++;

        printTab(_level);
        std::cout << "(call" << std::endl;

        printTab(_level + 1);
        if (n->getPreserveReturnValue())
            std::cout << "preserveReturnValue: true" << std::endl;
        else
            std::cout << "preserveReturnValue: false" << std::endl;

        n->getExpr()->accept(this);

        std::vector<IRExpr *> args = n->getArgs();
        for (unsigned i = 0; i < args.size(); i++)
            args[i]->accept(this);

        printTab(_level);
        std::cout << ")" << std::endl;

        _level--;
    }

    void visit(SeqIRExpr *n)
    {
        _level++;
        printTab(_level);
        std::cout << "(seq" << std::endl;

        std::vector<IRExpr *> elements = n->getElements();
        for (unsigned i = 0; i < elements.size(); i++)
            elements[i]->accept(this);

        printTab(_level);
        std::cout << ")" << std:: endl;
        _level--;
    }

    void visit(LabelIRExpr *n)
    {
        printTab(_level + 1);
        std::cout << "(label " << n->getName() << ")" << std::endl;
    }

    void visit(FunctionIRExpr *n)
    {
        _level++;

        printTab(_level);
        std::cout << "(function " << n->getName() << " [" << n->getFrameSize()
                  << "]" << std::endl;

        n->getBody()->accept(this);

        printTab(_level);
        std::cout << ")" << std::endl;

        _level--;
    }
};

class CountCallIRExprVisitor : public IRExprVisitor
{
    unsigned callExprCount; // Number of call expressions

public:
    CountCallIRExprVisitor() { callExprCount = 0; }

    unsigned getCallExprCount() { return callExprCount; }

    void visit(CallIRExpr *n) { callExprCount++; }
};

#endif
