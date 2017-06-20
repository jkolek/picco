// Abstract syntax tree - header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef ABSTRACT_SYNTAX_TREE_H
#define ABSTRACT_SYNTAX_TREE_H

#include <cstring>
#include <stack>
#include <queue>

#include "ASTNode.h"
#include "IRExprTree.h"

// Declaration flags
#define SCS_TYPEDEF 0x1
#define SCS_EXTERN 0x2
#define SCS_STATIC 0x4
#define SCS_AUTO 0x8
#define SCS_REGISTER 0x10

#define TQ_CONST 0x20
#define TQ_VOLATILE 0x40

#define UNLESS(x) if (!(x))
#define TYPE(x) x->type

// Match macros

#define AST_MATCH_NULL(n) (n == NULL_AST_NODE)

#define AST_MATCH_IDENT(n) (n->getKind() == NK_IDENT_NODE)

#define AST_MATCH_LIST(n) (n->getKind() == NK_LIST)

#define AST_MATCH_INTEGER_CONST(n)                                             \
    (n->getKind() == NK_INTEGER_CONST)

#define AST_MATCH_REAL_CONST(n)                                                \
    (n->getKind() == NK_REAL_CONST)

#define AST_MATCH_COMPLEX_CONST(n)                                             \
    (n->getKind() == NK_COMPLEX_CONST)

#define AST_MATCH_STRING_CONST(n)                                              \
    (n->getKind() == NK_STRING_CONST)

#define AST_MATCH_CHAR_CONST(n)                                                \
    (n->getKind() == NK_CHAR_CONST)

#define AST_MATCH_STRUCT_REF(n)                                                \
    (n->getKind() == NK_STRUCT_REF)

#define AST_MATCH_INDIRECT_REF(n)                                              \
    (n->getKind() == NK_INDIRECT_REF)

#define AST_MATCH_ARRAY_REF(n)                                                 \
    (n->getKind() == NK_ARRAY_REF)

#define AST_MATCH_STRUCT_TYPE(n)                                               \
    (n->getKind() == NK_STRUCT_TYPE)

#define AST_MATCH_VAR_DECL(n)                                                  \
    (n->getKind() == NK_VAR_DECL)

#define AST_MATCH_NOP_EXPR(n)                                                  \
    (n->getKind() == NK_NOP_EXPR)

#define AST_COMPOUND_STMT_STMTS(n)                                             \
    static_cast<CompoundStmtASTNode *>(n)->getStmts()

#define AST_COMPOUND_STMT_DECLS(n)                                             \
    static_cast<CompoundStmtASTNode *>(n)->getDecls()

#define AST_IDENT_VALUE(n) static_cast<IdentASTNode *>(n)->getValue()

#define AST_IDENT_LINE_NUM(n) static_cast<IdentASTNode *>(n)->getLineNum()

#define AST_LIST(n) static_cast<ListASTNode *>(n)

#define AST_LIST_ELEMENTS(n) static_cast<ListASTNode *>(n)->getElements()

#define AST_INTEGER_CONST_VALUE(n)                                             \
    static_cast<IntegerConstASTNode *>(n)->getValue()

#define AST_INTEGER_CONST_LINE_NUM(n)                                          \
    static_cast<IntegerConstASTNode *>(n)->getLineNum()

#define AST_ARRAY_REF_EXPR(n) static_cast<ArrayRefASTNode *>(n)->getExpr()

class ASTNode;

class AbstractSyntaxTree
{
    ASTNode *_root;
    IRExpr *_rootIR;

    SymbolTable *_stb;
    IRExprTree *_iret;

    Object_t _currentFuncDecl;
    std::stack<SeqIRExpr *> _sequences;

    int _errors;
    int _warnings;
    bool _opt;

    ASTNode *simplify(ASTNode *node);

public:
    Object_t currentRecordObj;
    IRExpr *currentIRExprFunc;

    ASTNode *voidTypeASTNode;
    ASTNode *integerTypeASTNode;
    ASTNode *unsignedTypeASTNode;
    ASTNode *shortTypeASTNode;
    ASTNode *charTypeASTNode;
    ASTNode *floatTypeASTNode;

    std::vector<StructTypeASTNode *> structTypes;
    std::vector<UnionTypeASTNode *> unionTypes;

    ASTNode *currentStmts;

    Object_t getCurrentFuncDecl()
    {
        return _currentFuncDecl;
    }

    void setCurrentFuncDecl(Object_t currentFuncDecl)
    {
        _currentFuncDecl = currentFuncDecl;
    }

    SeqIRExpr *getCurrentSeq()
    {
        return _sequences.top();
    }

    void pushCurrentSeq(SeqIRExpr *currentSeq)
    {
        _sequences.push(currentSeq);
    }

    void popCurrentSeq()
    {
        _sequences.pop();
    }

    // Label stack - wrapper methods

    std::stack<std::string> labelStack;
    void pushLabel(const char *lab) { labelStack.push(std::string(lab)); }
    void popLabel()
    {
        if (!labelStack.empty())
            labelStack.pop();
    }
    const char *getTopLabel()
    {
        if (!labelStack.empty())
            return labelStack.top().c_str();
        return nullptr;
    }

    // Continue label stack - wrapper methods

    std::stack<std::string> continueLabelStack;
    void pushContinueLabel(const char *lab)
    {
        continueLabelStack.push(std::string(lab));
    }
    void popContinueLabel()
    {
        if (!continueLabelStack.empty())
            continueLabelStack.pop();
    }
    const char *getTopContinueLabel()
    {
        if (!continueLabelStack.empty())
            return continueLabelStack.top().c_str();
        return nullptr;
    }

    // True label stack - wrapper methods

    std::stack<std::string> trueLabelStack;
    void pushTrueLabel(const char *lab)
    {
        trueLabelStack.push(std::string(lab));
    }
    void popTrueLabel()
    {
        if (!trueLabelStack.empty())
            trueLabelStack.pop();
    }
    const char *getTopTrueLabel()
    {
        if (!trueLabelStack.empty())
            return trueLabelStack.top().c_str();
        return nullptr;
    }

    // False label stack - wrapper methods

    std::stack<std::string> falseLabelStack;
    void pushFalseLabel(const char *lab)
    {
        falseLabelStack.push(std::string(lab));
    }
    void popFalseLabel()
    {
        if (!falseLabelStack.empty())
            falseLabelStack.pop();
    }
    const char *getTopFalseLabel()
    {
        if (!falseLabelStack.empty())
            return falseLabelStack.top().c_str();
        return nullptr;
    }

    std::queue<IRExpr *> exprQueue;
    std::queue<IRExpr *> & getExprQueue()
    {
        return exprQueue;
    }

    void error(const char *msg, int lineNum);
    void error(const char *msg, const char *s, int lineNum);
    void error(const char *msg, const char *s1, const char *s2, int lineNum);

    void warning(const char *msg, int lineNum);
    void warning(const char *msg, const char *s, int lineNum);

    void setRoot(ASTNode *Root) { _root = Root; }
    ASTNode *getRoot() { return _root; }

    void setOptimize(bool Opt) { _opt = Opt; }
    bool optimize() { return _opt; }

    void declare(ASTNode *node);
    void traverse(ASTNode *node);
    IRExpr *emitIR(ListASTNode *translationUnit);

    int getErrors() { return _errors; }
    int getWarnings() { return _warnings; }

    Type_t getStbType(ASTNode *typeASTNode);

    void printTree();

    // If there are no semantic errors this function emits the code and
    // returns 0, otherwise returns the number of semantic errors.
    int emitCode(const char *);

    SymbolTable *getSymbolTable() { return _stb; }
    CodeGenerator *getCodeGenerator() { return _iret->getCodeGenerator(); }
    IRExprTree *getIRExprTree() { return _iret; }

    StructTypeASTNode *createStructTypeASTNode(ASTNode *typeName,
                                               ASTNode *typeBody)
    {
        StructTypeASTNode *tmp = new StructTypeASTNode(typeName, typeBody);
        structTypes.push_back(tmp);
        return tmp;
    }

    AbstractSyntaxTree(SymbolTable *STB, char *target) : _stb(STB)
    {
        ASSIGN_AST_NODE_REF(voidTypeASTNode, new VoidTypeASTNode());
        ASSIGN_AST_NODE_REF(charTypeASTNode, new IntegralTypeASTNode(1, true));
        ASSIGN_AST_NODE_REF(shortTypeASTNode, new IntegralTypeASTNode(2, true));
        ASSIGN_AST_NODE_REF(integerTypeASTNode,
                            new IntegralTypeASTNode(4, true));
        ASSIGN_AST_NODE_REF(unsignedTypeASTNode,
                            new IntegralTypeASTNode(4, false));
        ASSIGN_AST_NODE_REF(floatTypeASTNode, new RealTypeASTNode(4, false));

        _currentFuncDecl = nullptr;
        currentIRExprFunc = NULL_IR_EXPR;
        currentRecordObj = nullptr;
        _errors = 0;
        _warnings = 0;

        _root = nullptr;
        _rootIR = nullptr;
        currentStmts = nullptr;
        _opt = false;

        _iret = new IRExprTree(target);
    }

    ~AbstractSyntaxTree()
    {
        delete _iret;

        if (_root)
            delete _root;

        DELETE_AST_NODE_REF(voidTypeASTNode);
        DELETE_AST_NODE_REF(charTypeASTNode);
        DELETE_AST_NODE_REF(shortTypeASTNode);
        DELETE_AST_NODE_REF(integerTypeASTNode);
        DELETE_AST_NODE_REF(unsignedTypeASTNode);
        DELETE_AST_NODE_REF(floatTypeASTNode);

        for (unsigned n = 0; n < structTypes.size(); n++)
            delete structTypes[n];
    }
};

#endif
