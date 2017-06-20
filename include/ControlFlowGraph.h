// Control flow graph - header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef CONTROL_FLOW_GRAPH_H
#define CONTROL_FLOW_GRAPH_H

#include "IRExpr.h"
#include "IRExprTree.h"
#include "PrintIRExprVisitor.h"

#include <vector>
#include <iostream>

class BasicBlock
{
private:
    std::vector<IRExpr *> exprs;
    std::vector<BasicBlock *> predecessors;
    std::vector<BasicBlock *> successors;

public:
    ~BasicBlock() {}

    std::vector<IRExpr *> &getElements() { return exprs; }

    void add(IRExpr *expr) { exprs.push_back(expr); }

    void addPredecessor(BasicBlock *pred) { predecessors.push_back(pred); }

    unsigned getPredecessorsCount() { return predecessors.size(); }

    void addSuccessor(BasicBlock *succ) { successors.push_back(succ); }

    const char *getEntryLabel()
    {
        if (exprs.size() > 0)
        {
            if (IR_MATCH_LABEL(exprs[0]))
                return IR_LABEL_VAL(exprs[0]);
        }
        return nullptr;
    }

    std::vector<NameIRExpr *> *getTargetLabels()
    {
        std::vector<NameIRExpr *> *labels = new std::vector<NameIRExpr *>();
        // TODO: Implement
        if (exprs.size() > 0)
        {
            IRExpr *lastInst = exprs[exprs.size() - 1];

            if (IR_MATCH_JUMP(lastInst))
            {
                labels->push_back(
                    static_cast<NameIRExpr *>(IR_JUMP_TARGET(lastInst)));
            }
            else if (IR_MATCH_CJUMP(lastInst))
            {
                if (IR_CJUMP_TLAB(lastInst) != nullptr)
                    labels->push_back(
                        static_cast<NameIRExpr *>(IR_CJUMP_TLAB(lastInst)));
                if (IR_CJUMP_FLAB(lastInst) != nullptr)
                    labels->push_back(
                        static_cast<NameIRExpr *>(IR_CJUMP_FLAB(lastInst)));
            }
        }
        return labels;
    }

    void print()
    {
        PrintIRExprVisitor piret;

        std::cout << "-----------------------------" << std::endl;
        std::cout << "  " << getEntryLabel() << std::endl;
        std::cout << "-----------------------------" << std::endl;

        for (unsigned i = 0; i < exprs.size(); i++)
            piret.visitIRExpr(exprs[i]);

        std::cout << std::endl << "Predecessors:" << std::endl;
        for (unsigned i = 0; i < predecessors.size(); i++)
            std::cout << "  " << predecessors[i]->getEntryLabel() << " ==>"
                      << std::endl;

        std::cout << std::endl << "Successors:" << std::endl;
        for (unsigned i = 0; i < successors.size(); i++)
            std::cout << "  " << successors[i]->getEntryLabel() << " ==>"
                      << std::endl;

        std::cout << std::endl;
    }
};

class ControlFlowGraph
{
private:
    std::vector<std::pair<FunctionIRExpr *, std::vector<BasicBlock *> *> >
        functions;
    std::vector<BasicBlock *> basic_block_pool;
    unsigned labelCount;

public:
    // Basic blocks
    std::vector<BasicBlock *> *createBasicBlocks(IRExpr *expr);
    void createEdges(std::vector<BasicBlock *> *basicBlocks);
    void removeUnusedBlocks(std::vector<BasicBlock *> *basicBlocks);
    void addFunction(FunctionIRExpr *func);
    SeqIRExpr *toSequence();
    void print();

    BasicBlock *createBasicBlock()
    {
        BasicBlock *bb = new BasicBlock();

        basic_block_pool.push_back(bb);
        return bb;
    }

    ControlFlowGraph() { labelCount = 0; }

    ~ControlFlowGraph()
    {
        for (unsigned i = 0; i < basic_block_pool.size(); i++)
            delete basic_block_pool[i];

        for (unsigned i = 0; i < functions.size(); i++)
            delete functions[i].second;
    }
};

#endif
