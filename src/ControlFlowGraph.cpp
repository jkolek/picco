// Control flow graph - implementation file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#include "../include/ControlFlowGraph.h"
#include "../include/IRExpr.h"

#include <vector>

/* Basic block - Creation algorithm
   Source: wikipedia.org

Input: A sequence of instructions (mostly three-address code). [5]
Output: A list of basic blocks with each three-address statement in exactly
        one block.

Step 1. Identify the leaders in the code. Leaders are instructions which come
under any of the following 3 categories:

  1. The first instruction is a leader.
  2. The target of a conditional or an unconditional goto/jump instruction is
     a leader.
  3. The instruction that immediately follows a conditional or an
     unconditional goto/jump instruction is a leader.

Step 2. Starting from a leader, the set of all following instructions until
and not including the next leader is the basic block corresponding to the
starting leader.  */

// Creates basic blocks from function bodies only.

// TODO: Before basic block creation IR must be transformed to canonical form.

std::vector<BasicBlock *> *ControlFlowGraph::createBasicBlocks(IRExpr *expr)
{
    assert(expr != NULL_IR_EXPR && expr->getKind() == IR_EK_SEQ);

    std::vector<IRExpr *> &elems =
        static_cast<SeqIRExpr *>(expr)->getElements();
    std::vector<BasicBlock *> *blocks = new std::vector<BasicBlock *>();
    BasicBlock *prev = NULL;
    unsigned i = 0;

    // Identify leaders of a basic block

    while (i < elems.size() && elems[i] == NULL_IR_EXPR)
        i++;

    if (i < elems.size())
        elems[i++]->setIsBBLeader(true);

    while (i < elems.size())
    {
        if (IR_MATCH_JUMP(elems[i]))
        {
            // TODO: Find target of the jump, it will be a leader ???
            if (i < elems.size())
                elems[i + 1]->setIsBBLeader(true);
        }
        else if (IR_MATCH_CJUMP(elems[i]))
        {
            // TODO: Find target of the jump, it will be a leader ???
            if (i < elems.size())
                elems[i + 1]->setIsBBLeader(true);
        }
        else if (IR_MATCH_CALL(elems[i]))
        {
            // TODO: Find target of the jump, it will be a leader ???
            if (i < elems.size())
                elems[i + 1]->setIsBBLeader(true);
        }
        else if (IR_MATCH_LABEL(elems[i]))
        {
            elems[i]->setIsBBLeader(true);
        }
        i++;
    }

    // Create basic blocks

    i = 0;
    while (i < elems.size())
    {
        if (elems[i] != NULL_IR_EXPR && elems[i]->getIsBBLeader())
        {
            BasicBlock *bb = createBasicBlock();

            // If there is no label add label at beginning of basic block.

            if (!IR_MATCH_LABEL(elems[i]))
            {
                char bblabel[LABEL_SIZE];

                snprintf(bblabel, LABEL_SIZE, "BB_%d", labelCount);
                labelCount++;
                bb->add(new LabelIRExpr(bblabel));
            }

            bb->add(elems[i]);
            i++;

            while (i < elems.size() && elems[i] != NULL_IR_EXPR &&
                   !elems[i]->getIsBBLeader())
            {
                bb->add(elems[i]);
                i++;
            }

            blocks->push_back(bb);

            if (prev != NULL)
                prev->addSuccessor(bb);

            prev = bb;

            continue;
        }
        i++;
    }

    if (blocks->size() > 0)
    {
        // Pseudo basic block representing ENTRY to function
        BasicBlock *entry = createBasicBlock();
        char bblabel[LABEL_SIZE];

        // snprintf(bblabel, LABEL_SIZE, "%s_entry", function_name);
        snprintf(bblabel, LABEL_SIZE, "__entry__");
        labelCount++;
        entry->add(new LabelIRExpr(bblabel));

        (*blocks)[0]->addPredecessor(entry);
    }

    return blocks;
}

static BasicBlock *findBasicBlock(const char *name,
                                  std::vector<BasicBlock *> *basicBlocks)
{
    for (unsigned n = 0; n < basicBlocks->size(); n++)
    {
        BasicBlock *bb = (*basicBlocks)[n];
        const char *label = bb->getEntryLabel();

        if (strcmp(label, name) == 0)
            return bb;
    }

    return NULL;
}

// Essentially this function adds successors to a created basic block of a
// function.
void ControlFlowGraph::createEdges(std::vector<BasicBlock *> *basicBlocks)
{
    // printf("=== %s\n", func->getName());
    for (unsigned n = 0; n < basicBlocks->size(); n++)
    {
        BasicBlock *bb = (*basicBlocks)[n];
        std::vector<NameIRExpr *> *targetLabels = bb->getTargetLabels();

        for (unsigned k = 0; k < targetLabels->size(); k++)
        {
            const char *name = (*targetLabels)[k]->getValue();
            BasicBlock *succ = findBasicBlock(name, basicBlocks);

            if (succ != NULL)
            {
                bb->addSuccessor(succ);
                succ->addPredecessor(bb);
            }
        }
        delete targetLabels;
    }
}

// Basic blocks without predecessors are unused.
void ControlFlowGraph::removeUnusedBlocks(
    std::vector<BasicBlock *> *basicBlocks)
{
    for (unsigned n = 0; n < basicBlocks->size(); n++)
    {
        BasicBlock *bb = (*basicBlocks)[n];

        if (bb->getPredecessorsCount() == 0)
        {
            // TODO: Remove this basic block
        }
    }
}

void ControlFlowGraph::addFunction(FunctionIRExpr *func)
{
    std::vector<BasicBlock *> *basicBlocks = createBasicBlocks(func->getBody());

    createEdges(basicBlocks);
    removeUnusedBlocks(basicBlocks);

    functions.push_back(std::make_pair(func, basicBlocks));
}

SeqIRExpr *ControlFlowGraph::toSequence()
{
    SeqIRExpr *seq = new SeqIRExpr();

    for (unsigned i = 0; i < functions.size(); i++)
    {
        FunctionIRExpr *func = functions[i].first;
        std::vector<BasicBlock *> *basicBlocks = functions[i].second;
        SeqIRExpr *body = new SeqIRExpr();

        for (unsigned n = 0; n < basicBlocks->size(); n++)
        {
            BasicBlock *bb = (*basicBlocks)[n];

            // body->add(bb->getElements());
        }

        func->setBody(body);
        seq->add(func);
    }

    return seq;
}

void ControlFlowGraph::print()
{
    for (unsigned i = 0; i < functions.size(); i++)
    {
        FunctionIRExpr *func = functions[i].first;
        std::vector<BasicBlock *> *basicBlocks = functions[i].second;

        std::cout << "===================================================="
                  << std::endl;
        std::cout << "  Control flow graph for: " << func->getName()
                  << std::endl;
        std::cout << "===================================================="
                          << std::endl << std::endl;

        for (unsigned n = 0; n < basicBlocks->size(); n++)
        {
            BasicBlock *bb = (*basicBlocks)[n];

            bb->print();
        }
    }
}
