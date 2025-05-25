// Abstract syntax tree - implementation file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

// TODO:
//   - Implement string constants.
//   - Implement passing arrays as parameters.
//   - Implement unsigned type.
//   - Global variable support in linker.
//   - Implement break in loops.
//   - FIXME: object is not detected as non-existing type!!!
//       struct object *list;
//   - FIXME: Function pointer calls.
//
//   - Replace:
//         static_cast<IdentASTNode *>(x)->getValue()
//       by
//         AST_IDENT_VALUE(x)
//
//         static_cast<IdentASTNode *>(x)->getLineNum()
//       by
//         AST_IDENT_LINE_NUM(x)
//
//     As well as other similar patterns.

#include "../include/AbstractSyntaxTree.h"
#include "../include/ASTNode.h"
#include "../include/CodeGenerator.h"
#include "../include/IRExpr.h"
#include "../include/IRExprTree.h"
#include "../include/PrintTreeVisitor.h"
#include "../include/TreeVisitor.h"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
#include <iostream>
#include <queue>

void AbstractSyntaxTree::error(const char *msg, int lineNum)
{
    std::cout << "line " << lineNum << ": error: " << msg << std::endl;
    _errors++;
    exit(1);
}

void AbstractSyntaxTree::error(const char *msg, const char *s, int lineNum)
{
    char msg1[MAXSTR];

    snprintf(msg1, MAXSTR, msg, s);
    std::cout << "line " << lineNum << ": error: " << msg1 << std::endl;
    _errors++;
    exit(1);
}

void AbstractSyntaxTree::error(const char *msg,
                               const char *s1,
                               const char *s2,
                               int lineNum)
{
    char msg1[MAXSTR];

    snprintf(msg1, MAXSTR, msg, s1, s2);
    std::cout << "line " << lineNum << ": error: " << msg1 << std::endl;
    _errors++;
    exit(1);
}

void AbstractSyntaxTree::warning(const char *msg, int lineNum)
{
    std::cout << "line " << lineNum << ": warning: " << msg << std::endl;
    _warnings++;
}

void AbstractSyntaxTree::warning(const char *msg, const char *s, int lineNum)
{
    char msg1[MAXSTR];

    snprintf(msg1, MAXSTR, msg, s);
    std::cout << "line " << lineNum << ": warning: " << msg1 << std::endl;
    _warnings++;
}

//
// Tree operations
//

void AbstractSyntaxTree::printTree()
{
    PrintTreeVisitor visitor;

    std::cout << std::endl << "Abstract syntax tree:" << std::endl << std::endl;
    visitor.visit(static_cast<ListASTNode *>(_root));
    std::cout << std::endl;
}

void AbstractSyntaxTree::declare(ASTNode *node) { node->declare(this); }

IRExpr *AbstractSyntaxTree::emitIR(ListASTNode *translationUnit)
{
    // Reset the scope.
    _stb->setTopScope(translationUnit->getScope());
    IRExpr *rootIR = translationUnit->emitIR(this);
    getCodeGenerator()->setStaticDataSize(translationUnit->getScope()->size);
    _stb->setTopScope(nullptr);
    return rootIR;
}

Type_t AbstractSyntaxTree::getStbType(ASTNode *typeASTNode)
{
    switch (typeASTNode->getKind())
    {
        case NK_VOID_TYPE:
            return _stb->voidType;
        case NK_INTEGRAL_TYPE:
        {
            IntegralTypeASTNode *intTypeASTNode =
                static_cast<IntegralTypeASTNode *>(typeASTNode);

            if (intTypeASTNode->getIsSigned())
            {
                switch (intTypeASTNode->getAlignment())
                {
                    case 1:
                        return _stb->charType;
                    case 2:
                        return _stb->shortType;
                    case 4:
                        return _stb->intType;
                    default:
                        return _stb->noType;
                }
            }
            else
            {
                switch (intTypeASTNode->getAlignment())
                {
                    // case 1:  return stb->charType;
                    // case 2:  return stb->shortType;
                    case 4:
                        return _stb->unsignedType;
                    default:
                        return _stb->noType;
                }
            }
        }
        break;
        case NK_REAL_TYPE:
        {
            RealTypeASTNode *realTypeASTNode =
                static_cast<RealTypeASTNode *>(typeASTNode);

            return realTypeASTNode->getIsDouble() ? _stb->doubleType
                                                  : _stb->floatType;
        }
        case NK_COMPLEX_TYPE:
            return _stb->noType;
        case NK_ENUMERAL_TYPE:
            return _stb->noType;
        case NK_BOOLEAN_TYPE:
            return _stb->noType;
        case NK_POINTER_TYPE:
        {
            Type_t type = _stb->allocType(T_POINTER);
            PointerTypeASTNode *ptrTypeASTNode =
                static_cast<PointerTypeASTNode *>(typeASTNode);

            type->baseType = getStbType(ptrTypeASTNode->getBaseType());
            type->size = 4; // Pointer type size is 4 bytes.
            return type;
        }
        case NK_REFERENCE_TYPE:
            return _stb->noType;
        case NK_FUNCTION_TYPE:
        {
            Type_t type = _stb->allocType(T_FUNCTION);
            FunctionDeclASTNode *funcTypeASTNode =
                static_cast<FunctionDeclASTNode *>(typeASTNode);

            type->funcType = getStbType(funcTypeASTNode->getType());
            type->size = 4;
            return type;
        }
        case NK_ARRAY_TYPE:
        {
            Type_t type = _stb->allocType(T_ARRAY);
            ArrayTypeASTNode *arrTypeASTNode =
                static_cast<ArrayTypeASTNode *>(typeASTNode);

            type->elemType = getStbType(arrTypeASTNode->getElementType());

            if (AST_MATCH_INTEGER_CONST(arrTypeASTNode->getExpr()))
                type->size = type->elemType->size *
                             AST_INTEGER_CONST_VALUE(arrTypeASTNode->getExpr());

            return type;
        }
        case NK_STRUCT_TYPE:
        {
            Type_t type = nullptr;
            // FIXME: At this point a record type should already be in the
            // symbol table.
            StructTypeASTNode *recTypeASTNode =
                static_cast<StructTypeASTNode *>(typeASTNode);

            if (AST_MATCH_IDENT(recTypeASTNode->getName()))
            {
                const char *recordName =
                    AST_IDENT_VALUE(recTypeASTNode->getName());
                unsigned recordLineNum =
                    AST_IDENT_LINE_NUM(recTypeASTNode->getName());
                Object_t obj = _stb->find(recordName);

                if (obj == _stb->noObj)
                    // Record type declaration is not found in symbol table.
                    error("struct type '%s' not declared", recordName,
                          recordLineNum);
                else
                    type = obj->type;
            }
            return type;
        }
        case NK_UNION_TYPE:
        {
            Type_t type = nullptr;
            // FIXME: At this point a union type should already be in the
            // symbol table.
            UnionTypeASTNode *unionTypeASTNode =
                static_cast<UnionTypeASTNode *>(typeASTNode);

            if (AST_MATCH_IDENT(unionTypeASTNode->getName()))
            {
                const char *unionName =
                    AST_IDENT_VALUE(unionTypeASTNode->getName());
                unsigned unionLineNum =
                    AST_IDENT_LINE_NUM(unionTypeASTNode->getName());
                Object_t obj = _stb->find(unionName);

                if (obj == _stb->noObj)
                    // Union type declaration is not found in symbol table.
                    error("union type '%s' not declared", unionName,
                          unionLineNum);
                else
                    type = obj->type;
            }
            return type;
        }
        case NK_UNKNOWN_TYPE:
            return _stb->noType;
        default:
            return _stb->noType;
    }
}

static bool isConstant(ASTNode *n)
{
    return AST_MATCH_INTEGER_CONST(n) || AST_MATCH_REAL_CONST(n) ||
           AST_MATCH_COMPLEX_CONST(n) || AST_MATCH_STRING_CONST(n) ||
           AST_MATCH_CHAR_CONST(n);
}

int AbstractSyntaxTree::emitCode(const char *output)
{
    if (!AST_MATCH_LIST(_root))
        return -1;

    /*ListASTNode *rootTLN = static_cast<ListASTNode *>(root);

    // TODO: Do some optimizations after simplification.
    //
    //  After simplification with optimization option turned on a statements
    //  like:
    //      x = a + b + c + d;
    //  are expanded to:
    //      t0 = a + b;
    //      t1 = t0 + c;
    //      x = t1 + d;
    //
    stb->setTopScope(rootTLN->getScope());

    // Reset the scope.
    stb->setTopScope(rootTLN->getScope());
    rootIR = emitIR(rootTLN);
    getCodeGenerator()->setStaticDataSize(rootTLN->getScope()->size);
    stb->setTopScope(NULL);

    // Jump optimization phase.
    iret->jumps(rootIR);
    iret->setPreserveReturnValueToFalse(rootIR);
    // iret->removeUnusedLabels(rootIR);

    iret->setRoot(rootIR);

    // FIXME: This function causes compiler to emit jump to 'main_epilogue'
    // for test 'while.c' even if 'main_epilogue' doesn't exists.
    // iret->removeLabelFollowedByLabel(rootIR);

    // printIRExprTree();

    // TODO: Create IR tree from the basic blocks.
    // if (opt)
    // iret->controlFlowAnalysis(rootIR);

    // Emit the machine code.
    iret->traverse(rootIR);

    if (errors == 0)
        getCodeGenerator()->write(output);*/

    return _errors;
}

//
// ASTNode traverse methods
//

static IRExpr *
emitMemoryAccess(Object_t obj, AbstractSyntaxTree *ast, int lineNum)
{
    IRExprTree *iret = ast->getIRExprTree();
    SymbolTable *stb = ast->getSymbolTable();
    IRExpr *var = NULL_IR_EXPR;

    if (obj == stb->noObj)
        ast->error("unknown identifier '%s'", obj->name, lineNum);

    if (obj->level == 0 || obj->kind == OBJ_VAR_STATIC)
    {
        var = IR_MEM(iret->stbToIRType(obj->type),
                     IR_BINOP(IR_i32, IRBK_PLUS, IR_TEMP(IR_i32, "GP"),
                              IR_CONST(IR_i32, obj->adr)));
    }
    else
    {
        // FIXME:
        // - Add support for local variables in regs A0, A1, A2 and A3.
        // - If registers $a0-$a3 are holding parameter values they need
        //   to be preserved as well.

#ifdef GNU_ABI
        if (obj->kind == OBJ_PAR && obj->parIndex < 4)
        {
            const char *reg = NULL;

            switch (obj->parIndex)
            {
                case 0:
                    reg = "A0";
                    break;
                case 1:
                    reg = "A1";
                    break;
                case 2:
                    reg = "A2";
                    break;
                case 3:
                    reg = "A3";
                    break;
            }
            var = IR_TEMP(IR_i32, reg);
        }
        else
        {
            var = IR_MEM(iret->stbToIRType(obj->type),
                         IR_BINOP(IR_i32, IRBK_PLUS, IR_TEMP(IR_i32, "SP"),
                                  IR_CONST(IR_i32, new FrameOffset(obj->adr))));
        }
#else
        var = IR_MEM(iret->stbToIRType(obj->type),
                     IR_BINOP(IR_i32, IRBK_PLUS, IR_TEMP(IR_i32, "SP"),
                              IR_CONST(IR_i32, new FrameOffset(obj->adr))));
#endif
    }

    return var;
}

Type_t IdentASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb = ast->getSymbolTable();
    Type_t tp = nullptr;

    if (strcmp(_value, "NULL") == 0)
    {
        tp = stb->nullType;
    }
    else
    {
        Object_t obj = stb->find(_value);

        if (obj != stb->noObj)
            tp = obj->type;
        else
            ast->error("unknown identifier '%s'", _value, lineNum);
    }

    return tp;
}

IRExpr *IdentASTNode::emitIR(AbstractSyntaxTree *ast)
{
    Object_t obj;
    SymbolTable *stb;

    // FIXME: This is a hack, this should be implemented in a different way.
    if (strcmp(_value, "NULL") == 0)
        return IR_CONST(IR_i32, 0);

    stb = ast->getSymbolTable();
    obj = stb->find(_value);
    if (obj == stb->noObj)
        ast->error("unknown identifier '%s'", _value, lineNum);

    // Enums
    if (obj->kind == OBJ_CON)
        return IR_CONST(IR_i32, obj->ival);

    return emitMemoryAccess(obj, ast, lineNum);
}

Type_t IntegerConstASTNode::checkType(AbstractSyntaxTree *ast)
{
    return ast->getSymbolTable()->intType;
}

IRExpr *IntegerConstASTNode::emitIR(AbstractSyntaxTree *ast)
{
    return IR_CONST(IR_i32, _value);
}

Type_t RealConstASTNode::checkType(AbstractSyntaxTree *ast)
{
    return ast->getSymbolTable()->floatType;
}

IRExpr *RealConstASTNode::emitIR(AbstractSyntaxTree *ast)
{
    return IR_CONST(IR_f32, _value);
}

Type_t StringConstASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb = ast->getSymbolTable();
    Type_t type = stb->allocType(T_POINTER);

    type->baseType = ast->getSymbolTable()->charType;
    return type;
}

IRExpr *StringConstASTNode::emitIR(AbstractSyntaxTree *ast)
{
    // CodeGenerator *gen = ast->getCodeGenerator();
    IRExprTree *iret = ast->getIRExprTree();
    unsigned labc = iret->getLabelCount();
    char label[LABEL_SIZE];

    if (ast->currentIRExprFunc != NULL_IR_EXPR)
    {
        snprintf(label, LABEL_SIZE, "$LC_%d", labc);
        static_cast<FunctionIRExpr *>(ast->currentIRExprFunc)
            ->addStringConst(label, _value);
    }

    // TODO: Add string value to .rodata section
    // Get an position of the string.
    /*unsigned pos = gen->addStringToReadOnlyData(value);
    return IR_BINOP(IR_i32,
                    IRBK_PLUS,
                    IR_NAME(".rodata"),
                    IR_CONST(IR_i32, pos));*/

    return IR_CONST(IR_i32, IR_NAME(label));
}

Type_t CharConstASTNode::checkType(AbstractSyntaxTree *ast)
{
    return ast->getSymbolTable()->charType;
}

IRExpr *CharConstASTNode::emitIR(AbstractSyntaxTree *ast)
{
    return IR_CONST(IR_i8, _value);
}

Type_t SizeOfExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    return ast->getSymbolTable()->intType;
}

IRExpr *SizeOfExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    int size = 0;

    if (AST_MATCH_STRUCT_TYPE(_expr))
    {
        StructTypeASTNode *recordType = static_cast<StructTypeASTNode *>(_expr);

        if (AST_MATCH_IDENT(recordType->getName()))
        {
            SymbolTable *stb = ast->getSymbolTable();
            const char *typeName = AST_IDENT_VALUE(recordType->getName());
            unsigned typeLineNum = AST_IDENT_LINE_NUM(recordType->getName());
            Object_t obj = stb->find(typeName);

            if (obj == stb->noObj)
                ast->error("type '%s' does not exist", typeName, typeLineNum);

            if (obj->kind != OBJ_TYPE)
                ast->error("object '%s' is not a type", typeName, typeLineNum);

            size = obj->type->size;
        }
    }
    else if (_expr->isType())
    {
        Type_t tp = ast->getStbType(_expr);

        size = tp->size;
    }

    /*if (AST_MATCH_IDENT(expr))
    {
        IdentASTNode *typeName = static_cast<IdentASTNode *>(expr);
        Object_t obj = stb->find(typeName->getValue());
        if (obj != stb->noObj)
        {
            if (obj->kind != OBJ_TYPE)
                ast->error("object '%s' is not a type",
                        typeName->getValue(),
                        typeName->getLineNum());
            x->mode = I_CONST;
            x->ival = obj->type->size;
            x->type = stb->intType;
        }
        else
        {
            ast->error("type '%s' does not exist",
                       typeName->getValue(),
                       typeName->getLineNum());
        }
    }*/

    return IR_CONST(IR_i32, size);
}

static bool isStaticVarDecl(ASTNode *n)
{
    return (
        AST_MATCH_VAR_DECL(n) &&
        ((static_cast<VarDeclASTNode *>(n)->getFlags() & SCS_STATIC) != 0x0));
}

IRExpr *FunctionDeclASTNode::emitIR(AbstractSyntaxTree *ast)
{
    char funcEpilogueLabel[LABEL_SIZE];
    const char *fnName;
    LabelIRExpr *lab;
    FunctionIRExpr *func;
    SeqIRExpr *seq;
    bool hasLocals, isNopExpr;
    Scope_t prevScope;
    ASTNode *stmts;
    SymbolTable *stb;
    Object_t obj;

    assert(!AST_MATCH_NULL(_name) && "Function declaration name is NULL.");

    if (isForwardDeclaration())
        return NULL_IR_EXPR;

    seq = IR_SEQ();
    hasLocals = false;
    stb = ast->getSymbolTable();
    fnName = AST_IDENT_VALUE(_name);
    obj = stb->find(fnName);

    if (obj != stb->noObj)
        ast->setCurrentFuncDecl(obj);

    if (AST_MATCH_LIST(_prms))
    {
        hasLocals = true;
        std::vector<ASTNode *> &tmpVec = AST_LIST_ELEMENTS(_prms);
        ast->getCurrentFuncDecl()->prmc = tmpVec.size();
    }

    prevScope = stb->getTopScope();
    stb->setTopScope(_scope);

    func = IR_FUNCTION(fnName, stb->getTopScope()->size /* frameSize */);
    ast->currentIRExprFunc = func;

    if (!AST_MATCH_NULL(AST_COMPOUND_STMT_DECLS(_body)))
        hasLocals = true;

    if (hasLocals)
    {
        Scope_t s = stb->getTopScope();

        if (s != nullptr)
        {
            /*#ifdef GNU_ABI
                        int i = 0;
            #endif*/

            for (Object_t p = s->locals; p != nullptr; p = p->next)
            {
                /*#ifdef GNU_ABI
                                if (i < ast->currentFuncDecl->prmc && i < 4)
                                {
                                    char *reg = NULL;

                                    switch (i)
                                    {
                                        case 0: reg = "A0"; break;
                                        case 1: reg = "A1"; break;
                                        case 2: reg = "A2"; break;
                                        case 3: reg = "A3"; break;
                                    }
                                    func->addInReg(IR_TEMP(IR_i32, reg));
                                }
                                else
                                {
                                    func->addInFrame(p->adr, p->type->size);
                                }
                                i++;
                #else*/
                func->addInFrame(p->adr, p->type->size);
                //#endif
            }
        }
    }

    stmts = AST_COMPOUND_STMT_STMTS(_body);
    std::vector<ASTNode *> &tmpVec = AST_LIST_ELEMENTS(stmts);

    isNopExpr = AST_MATCH_NOP_EXPR(tmpVec[0]) ? true : false;

    if (isNopExpr)
        goto end_func;

    // Emit code for an initializers.
    if (!AST_MATCH_NULL(AST_COMPOUND_STMT_DECLS(_body)))
    {
        std::vector<ASTNode *> &tmpVec =
            AST_LIST_ELEMENTS(AST_COMPOUND_STMT_DECLS(_body));

        for (unsigned i = 0; i < tmpVec.size(); i++)
        {
            // CodeGenerator *gen = ast->getCodeGenerator();

            // TODO: Check if this local variable is static.

            /*if (AST_MATCH_VAR_DECL(tmpVec[i]) &&
                static_cast<VarDeclASTNode *>(tmpVec[i])->getFlags() &
                SCS_STATIC != 0x0)
            {
                VarDeclASTNode *varDecl
                    = static_cast<VarDeclASTNode *>(tmpVec[i]);
                IdentASTNode *varName
                    = static_cast<IdentASTNode *>(varDecl->getName());
                if (stb->getLevel() == 0 &&
                    varDecl->getInit() != NULL_AST_NODE &&
                    !isConstant(varDecl->getInit()))
                {
                    ast->error("initializer element of '%s' is not constant",
                            varName->getValue(), varName->getLineNum());
                }
                else
                {
                    int value = 0;
                    Object_t obj = stb->find(varName->getValue());
                    if (AST_MATCH_INTEGER_CONST(varDecl->getInit()))
                    {
                        value = AST_INTEGER_CONST_VALUE(varDecl->getInit());
                    }
                    if (obj != stb->noObj)
                    gen->addToDataSection(obj->adr, value);
                }
            }
            else
            {*/
            if (!isStaticVarDecl(tmpVec[i]))
                seq->add(tmpVec[i]->emitIR(ast));
        }
    }

    ast->pushCurrentSeq(seq);
    seq->add(stmts->emitIR(ast));
    ast->popCurrentSeq();
    snprintf(funcEpilogueLabel, LABEL_SIZE, "%s_epilogue", fnName);
    lab = IR_LABEL(funcEpilogueLabel);
    seq->add(lab);

end_func:
    stb->setTopScope(prevScope);
    ast->setCurrentFuncDecl(nullptr);
    ast->currentIRExprFunc = NULL_IR_EXPR;
    func->setBody(seq);

    return func;
}

IRExpr *BreakStmtASTNode::emitIR(AbstractSyntaxTree *ast)
{
    return IR_JUMP(IR_NAME(ast->getTopLabel()));
}

IRExpr *CaseLabelASTNode::emitIR(AbstractSyntaxTree *ast)
{
    return NULL_IR_EXPR;
}

IRExpr *CompoundStmtASTNode::emitIR(AbstractSyntaxTree *ast)
{
    SeqIRExpr *seq = IR_SEQ();

    ast->pushCurrentSeq(seq);

    if (_stmts != NULL_AST_NODE)
        seq->add(_stmts->emitIR(ast));

    std::queue<IRExpr *> exprQueue = ast->getExprQueue();
    while (!exprQueue.empty())
    {
        seq->add(exprQueue.front());
        exprQueue.pop();
    }

    ast->popCurrentSeq();

    return seq;
}

IRExpr *ContinueStmtASTNode::emitIR(AbstractSyntaxTree *ast)
{
    return IR_JUMP(IR_NAME(ast->getTopContinueLabel()));
}

// FIXME: Push 'true_lab' and 'false_lab' for logical 'and', 'or' and 'not'.
IRExpr *DoStmtASTNode::emitIR(AbstractSyntaxTree *ast)
{
    SeqIRExpr *seq = IR_SEQ();
    IRExpr *cond;
    char loop_lab[LABEL_SIZE];
    // char true_lab[LABEL_SIZE];
    char false_lab[LABEL_SIZE];
    IRExprTree *iret = ast->getIRExprTree();
    unsigned labc = iret->getLabelCount();

    snprintf(loop_lab, LABEL_SIZE, "loop_lab_%d", labc);
    snprintf(false_lab, LABEL_SIZE, "false_lab_%d", labc);

    // Expression should have scalar type
    //UNLESS(stb->isScalarType(x->type))
    //    ast->error("invalid condition type of 'do-while' statement", lineNum);

    // Mark label for backward jump/branch
    seq->add(IR_LABEL(loop_lab));

    // This will be used as 'continue label'
    ast->pushContinueLabel(loop_lab);
    ast->pushTrueLabel(loop_lab);

    // This will be used for break as 'done label'
    ast->pushLabel(false_lab);
    ast->pushFalseLabel(false_lab);

    // do-while body
    seq->add(_body->emitIR(ast));

    cond = _condition->emitIR(ast);
    if (IR_MATCH_RELOP(cond))
    {
        seq->add(IR_CJUMP(cond, NULL_IR_EXPR, IR_NAME(false_lab)));
    }
    else if (IR_MATCH_SEQ(cond))
    {
        seq->add(cond);
        // seq->add(IR_JUMP(IR_NAME(false_lab)));
    }
    else if (cond->getKind() != IR_EK_CJUMP)
    {
        seq->add(IR_CJUMP(IR_RELOP(IRRK_NE, cond, IR_CONST(IR_i32, 0)),
                          NULL_IR_EXPR, IR_NAME(false_lab)));
    }

    // Condition
    /*seq->add(CJUMP(condition->emitIR(ast),
                    NULL,
                    IR_NAME(false_lab)));*/

    seq->add(IR_JUMP(IR_NAME(loop_lab)));
    seq->add(IR_LABEL(false_lab));
    // Remove 'done label'
    ast->popLabel();
    ast->popTrueLabel();
    ast->popFalseLabel();
    ast->popContinueLabel();

    return seq;
}

IRExpr *ForStmtASTNode::emitIR(AbstractSyntaxTree *ast)
{
    // for (init; condition; step) body

    SeqIRExpr *seq = IR_SEQ();
    char loop_lab[LABEL_SIZE];
    char false_lab[LABEL_SIZE];
    IRExprTree *iret = ast->getIRExprTree();
    unsigned labc = iret->getLabelCount();

    snprintf(loop_lab, LABEL_SIZE, "loop_lab_%d", labc);
    snprintf(false_lab, LABEL_SIZE, "false_lab_%d", labc);

    // Expression should be of a scalar type
    /*UNLESS(stb->isScalarType(x->type))
        ast->error("invalid condition type of 'while' statement", lineNum);*/

    // The init
    if (_init != NULL_AST_NODE)
        seq->add(_init->emitIR(ast));

    // Mark label for backward jump/branch
    seq->add(IR_LABEL(loop_lab));
    // Condition
    seq->add(
        IR_CJUMP(_condition->emitIR(ast), NULL_IR_EXPR, IR_NAME(false_lab)));
    // Loop body
    seq->add(_body->emitIR(ast));
    // The step
    if (_step != NULL_AST_NODE)
        seq->add(_step->emitIR(ast));
    seq->add(IR_JUMP(IR_NAME(loop_lab)));
    seq->add(IR_LABEL(false_lab));

    return seq;
}

IRExpr *GotoStmtASTNode::emitIR(AbstractSyntaxTree *ast)
{
    SymbolTable *stb = ast->getSymbolTable();
    Object_t obj = stb->find(AST_IDENT_VALUE(_label));

    if (obj != stb->noObj)
    {
        // TODO: Add label to the list of labels and when exiting function
        // check if label was defined.
    }

    return IR_JUMP(IR_NAME(AST_IDENT_VALUE(_label)));
}

IRExpr *IfStmtASTNode::emitIR(AbstractSyntaxTree *ast)
{
    SeqIRExpr *seq = IR_SEQ();
    IRExpr *cond;
    char true_lab[LABEL_SIZE];
    char else_lab[LABEL_SIZE];
    char end_lab[LABEL_SIZE];
    IRExprTree *iret = ast->getIRExprTree();
    unsigned labc = iret->getLabelCount();

    snprintf(true_lab, LABEL_SIZE, "true_lab_%d", labc);
    snprintf(else_lab, LABEL_SIZE, "else_lab_%d", labc);
    snprintf(end_lab, LABEL_SIZE, "end_lab_%d", labc);

    // FIXME:
    //  If condition expression is in right side of assign expression:
    //  x = (y <= 100) && (y != 50);
    //  then true_lab and else_lab are missing, and they are required
    //  in LogAndExprASTNode, LogOrExprASTNode as prev_true_lab and
    //  prev_false_lab.
    ast->pushTrueLabel(true_lab);
    ast->pushFalseLabel(else_lab);

    // Condition
    cond = _condition->emitIR(ast);

    if (IR_MATCH_RELOP(cond))
        seq->add(IR_CJUMP(cond, NULL_IR_EXPR, IR_NAME(else_lab)));
    else if (IR_MATCH_SEQ(cond))
        seq->add(cond);
    // seq->add(IR_JUMP(IR_NAME(else_lab)));
    else if (cond->getKind() != IR_EK_CJUMP)
        seq->add(IR_CJUMP(IR_RELOP(IRRK_NE, cond, IR_CONST(IR_i32, 0)),
                          NULL_IR_EXPR, IR_NAME(else_lab)));

    ast->popTrueLabel();
    ast->popFalseLabel();

    seq->add(IR_LABEL(true_lab));
    seq->add(_thenClause->emitIR(ast));

    // 'else' clause
    if (_elseClause != NULL_AST_NODE)
    {
        seq->add(IR_JUMP(IR_NAME(end_lab)));
        seq->add(IR_LABEL(else_lab));
        seq->add(_elseClause->emitIR(ast));
        seq->add(IR_LABEL(end_lab));
    }
    else
        seq->add(IR_LABEL(else_lab));

    return seq;
}

IRExpr *LabelStmtASTNode::emitIR(AbstractSyntaxTree *ast)
{
    SymbolTable *stb = ast->getSymbolTable();
    SeqIRExpr *seq = IR_SEQ();
    const char *labelName = AST_IDENT_VALUE(_label);
    unsigned labelLineNum = AST_IDENT_LINE_NUM(_label);
    Object_t obj = stb->insert(labelName, OBJ_LAB, nullptr);

    if (obj == stb->noObj)
        ast->error("identifier '%s' already exist", labelName, labelLineNum);

    seq->add(IR_LABEL(labelName));
    seq->add(_stmt->emitIR(ast));

    return seq;
}

Type_t ReturnStmtASTNode::checkType(AbstractSyntaxTree *ast)
{
    if (AST_MATCH_NULL(_expr))
        return nullptr;

    SymbolTable *stb;
    Type_t t1 = ast->getCurrentFuncDecl()->type;
    Type_t t2 = _expr->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    stb = ast->getSymbolTable();
    // Type constraints are same as in assignment.
    UNLESS((stb->isArithmeticType(t1) && stb->isArithmeticType(t2)) ||
           (t1->kind == T_POINTER && t2->baseType == stb->noType) ||
           (t2->kind == T_POINTER && t1->baseType == stb->noType) ||
           (t1->kind == T_POINTER && t2 == stb->nullType) ||
           (stb->equalFunctionPointerTypes(t1, t2)) ||
           (stb->equalBasePointerTypes(t1, t2)))
    ast->error("invalid return type", lineNum);

    return t1;
}

IRExpr *ReturnStmtASTNode::emitIR(AbstractSyntaxTree *ast)
{
    // TODO: Make this return code generation for the main function as well.
    char funcEpilogueLabel[LABEL_SIZE];
    SeqIRExpr *seq;
    IRExpr *e = NULL_IR_EXPR;

    if (!AST_MATCH_NULL(_expr))
    {
        Type_t tp;
        SymbolTable *stb = ast->getSymbolTable();

        if (ast->getCurrentFuncDecl()->type == stb->voidType)
            ast->error(
                "return statement with a value, in function returning void",
                lineNum);

        tp = checkType(ast);

        if (tp == nullptr)
            return NULL_IR_EXPR;

        e = _expr->emitIR(ast);
    }

    seq = IR_SEQ();
    if (e != NULL_IR_EXPR)
        seq->add(IR_MOVE(IR_i32, IR_TEMP(IR_i32, "RV"), e));

    snprintf(funcEpilogueLabel, LABEL_SIZE, "%s_epilogue", ast->getCurrentFuncDecl()->name);
    seq->add(IR_JUMP(IR_NAME(funcEpilogueLabel)));

    return seq;
}

IRExpr *SwitchStmtASTNode::emitIR(AbstractSyntaxTree *ast)
{
    return NULL_IR_EXPR;
}

IRExpr *WhileStmtASTNode::emitIR(AbstractSyntaxTree *ast)
{
    SeqIRExpr *seq = IR_SEQ();
    IRExpr *cond;
    char loop_lab[LABEL_SIZE];
    // char true_lab[LABEL_SIZE];
    char false_lab[LABEL_SIZE];
    IRExprTree *iret = ast->getIRExprTree();
    unsigned labc = iret->getLabelCount();

    snprintf(loop_lab, LABEL_SIZE, "loop_lab_%d", labc);
    snprintf(false_lab, LABEL_SIZE, "false_lab_%d", labc);

    // Expression should have scalar type
    //UNLESS(stb->isScalarType(x->type))
    //    ast->error("invalid condition type of 'while' statement", lineNum);

    seq->add(IR_LABEL(loop_lab));

    // This will be used as 'continue label'
    ast->pushContinueLabel(loop_lab);
    ast->pushTrueLabel(loop_lab);

    // This will be used for break as 'done label'
    ast->pushLabel(false_lab);
    ast->pushFalseLabel(false_lab);

    cond = _condition->emitIR(ast);

    if (IR_MATCH_RELOP(cond))
        seq->add(IR_CJUMP(cond, NULL_IR_EXPR, IR_NAME(false_lab)));
    else if (IR_MATCH_SEQ(cond))
        seq->add(cond);
    // seq->add(IR_JUMP(IR_NAME(false_lab)));
    else if (cond->getKind() != IR_EK_CJUMP)
        seq->add(IR_CJUMP(IR_RELOP(IRRK_NE, cond, IR_CONST(IR_i32, 0)),
                          NULL_IR_EXPR, IR_NAME(false_lab)));

    // Condition
    // seq->add(CJUMP(condition->emitIR(ast),
    //               NULL_IR_EXPR,
    //               IR_NAME(false_lab)));

    // Loop body
    seq->add(_body->emitIR(ast));
    seq->add(IR_JUMP(IR_NAME(loop_lab)));
    seq->add(IR_LABEL(false_lab));
    // Remove 'done label'
    ast->popLabel();
    ast->popTrueLabel();
    ast->popFalseLabel();
    ast->popContinueLabel();

    return seq;
}

Type_t CastExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb = ast->getSymbolTable();
    Type_t exprType = _expr->checkType(ast);
    Type_t newType = ast->getStbType(_type);

    // TODO: Add names of types to the error message.
    UNLESS((newType == stb->noType) ||
           (stb->isScalarType(exprType) && stb->isScalarType(newType)))
    ast->error("invalid conversion", _expr->getLineNum());

    return newType;
}

IRExpr *CastExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    checkType(ast);
    return _expr->emitIR(ast);
}

Type_t BitNotExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t exprType = _expr->checkType(ast);

    assert(exprType != NULL);

    stb = ast->getSymbolTable();
    UNLESS(stb->isIntegralType(exprType))
    ast->error("wrong type argument to bit-complement", lineNum);

    return exprType;
}

// NOT x <=> x XOR -1
IRExpr *BitNotExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret;
    IRExpr *x;
    Type_t tp = checkType(ast);

    if (tp == nullptr)
        return NULL_IR_EXPR;

    x = _expr->emitIR(ast);

    iret = ast->getIRExprTree();
    return IR_BINOP(iret->stbToIRType(tp), IRBK_XOR, x, IR_CONST(IR_i32, -1));
}

Type_t LogNotExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t exprType;

    exprType = _expr->checkType(ast);

    assert(exprType != nullptr);

    stb = ast->getSymbolTable();
    UNLESS(stb->isScalarType(exprType))
    ast->error("wrong type argument to unary exclamation mark", lineNum);

    return exprType;
}

// TODO: Complete and test!!!
IRExpr *LogNotExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret;
    SeqIRExpr *seq;
    char true_lab[LABEL_SIZE];
    const char *prev_true_lab, *prev_false_lab;
    unsigned labc;
    IRExpr *x;
    Type_t tp = checkType(ast);

    if (tp == nullptr)
        return NULL_IR_EXPR;

    prev_true_lab = ast->getTopTrueLabel();
    prev_false_lab = ast->getTopFalseLabel();
    iret = ast->getIRExprTree();
    labc = iret->getLabelCount();

    snprintf(true_lab, LABEL_SIZE, "true_lab_%d", labc);
    ast->pushTrueLabel(true_lab);

    seq = IR_SEQ();
    x = _expr->emitIR(ast);
    if (IR_MATCH_SEQ(x))
        seq->add(x);
    else
        seq->add(IR_CJUMP(IR_RELOP(IRRK_EQ, x, IR_CONST(IR_i32, 0)),
                          IR_NAME(prev_true_lab), IR_NAME(prev_false_lab)));

    seq->add(IR_LABEL(true_lab));
    ast->popTrueLabel();

    return seq;
}

Type_t PredecrementExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t exprType = _expr->checkType(ast);

    assert(exprType != nullptr);

    stb = ast->getSymbolTable();
    UNLESS(stb->isIntegralType(exprType) || stb->isPointerType(exprType))
        ast->error("wrong type argument to predecrement expression", lineNum);

    return exprType;
}

IRExpr *PredecrementExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    SeqIRExpr *seq = ast->getCurrentSeq();
    Type_t tp = checkType(ast);
    IRExpr *x = _expr->emitIR(ast);
    unsigned n = tp->kind == T_POINTER ? tp->baseType->size : 1;

    // Add this before the whole statement IR subtree.
    seq->add(IR_MOVE(IR_i32, x,
                     IR_BINOP(IR_i32, IRBK_MINUS, x, IR_CONST(IR_i32, n))));

    return x;
}

Type_t PreincrementExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t exprType = _expr->checkType(ast);

    assert(exprType != nullptr);

    stb = ast->getSymbolTable();
    UNLESS(stb->isIntegralType(exprType) || stb->isPointerType(exprType))
        ast->error("wrong type argument to preincrement expression", lineNum);

    return exprType;
}

IRExpr *PreincrementExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    SeqIRExpr *seq = ast->getCurrentSeq();
    Type_t tp = checkType(ast);
    IRExpr *x = _expr->emitIR(ast);
    unsigned n = tp->kind == T_POINTER ? tp->baseType->size : 1;

    // Add this before the whole statement IR subtree.
    seq->add(IR_MOVE(IR_i32, x,
                     IR_BINOP(IR_i32, IRBK_PLUS, x, IR_CONST(IR_i32, n))));

    return x;
}

Type_t PostdecrementExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t exprType = _expr->checkType(ast);

    assert(exprType != nullptr);

    stb = ast->getSymbolTable();
    UNLESS(stb->isIntegralType(exprType) || stb->isPointerType(exprType))
        ast->error("wrong type argument to postdecrement expression", lineNum);

    return exprType;
}

IRExpr *PostdecrementExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    SeqIRExpr *seq = IR_SEQ();
    Type_t tp = checkType(ast);
    IRExpr *x = _expr->emitIR(ast);
    unsigned n = tp->kind == T_POINTER ? tp->baseType->size : 1;

    // Push this to a queue and pop and add to the sequence after the whole
    // statement IR subtree is emitted.
    /*std::queue<IRExpr *> exprQueue = ast->getExprQueue();
    exprQueue.push(IR_MOVE(IR_i32, x,
                   IR_BINOP(IR_i32, IRBK_MINUS, x, IR_CONST(IR_i32, sz))));

    return x;*/

    seq->add(IR_MOVE(IR_i32, x,
                     IR_BINOP(IR_i32, IRBK_MINUS, x, IR_CONST(IR_i32, n))));

    return seq;
}

Type_t PostincrementExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t exprType = _expr->checkType(ast);

    assert(exprType != nullptr);

    stb = ast->getSymbolTable();
    UNLESS(stb->isIntegralType(exprType) || stb->isPointerType(exprType))
        ast->error("wrong type argument to postdecrement expression", lineNum);

    return exprType;
}

IRExpr *PostincrementExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    SeqIRExpr *seq = IR_SEQ();
    Type_t tp = checkType(ast);
    IRExpr *x = _expr->emitIR(ast);
    unsigned n = tp->kind == T_POINTER ? tp->baseType->size : 1;

    // Push this to a queue and pop and add to the sequence after the whole
    // statement IR subtree is emitted.
    /*std::queue<IRExpr *> exprQueue = ast->getExprQueue();

    exprQueue.push(IR_MOVE(IR_i32, x,
                           IR_BINOP(IR_i32, IRBK_PLUS, x, IR_CONST(IR_i32, sz))));

    return x;*/

    seq->add(IR_MOVE(IR_i32, x,
                     IR_BINOP(IR_i32, IRBK_PLUS, x, IR_CONST(IR_i32, n))));

    return seq;
}

Type_t AddrExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    Type_t tp;
    SymbolTable *stb = ast->getSymbolTable();
    const char *name = AST_IDENT_VALUE(_expr);
    unsigned nameLineNum = AST_IDENT_LINE_NUM(_expr);
    Object_t obj = stb->find(name);

    if (obj == stb->noObj)
        ast->error("object '%s' does not exist", name, nameLineNum);

    tp = stb->allocType(T_POINTER);

    // FIXME: This is not a general case.
    if (obj->kind == OBJ_FUNC)
    {
        tp->baseType = stb->allocType(T_FUNCTION);
        tp->baseType->funcType = obj->type;
    }
    else if (obj->type->kind == T_ARRAY)
        tp->baseType = obj->type->elemType;
    else
        tp->baseType = obj->type;

    return tp;
}

IRExpr *AddrExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    // ADDR_EXPR represents a pointer to an object.
    //  Example:
    //      p = &i; --> &i is ADDR_EXPR.

    IRExpr *adr;
    SymbolTable *stb = ast->getSymbolTable();
    const char *name = AST_IDENT_VALUE(_expr);
    unsigned nameLineNum = AST_IDENT_LINE_NUM(_expr);
    Object_t obj = stb->find(name);

    if (obj == stb->noObj)
        ast->error("object '%s' does not exist", name, nameLineNum);

    // FIXME: BUG!!!! Compiler generates addiu sp, sp, num
    // FIXME: This is not a general case.
    if (obj->kind == OBJ_FUNC)
    {
        adr = IR_CONST(IR_i32, IR_NAME(obj->name));
    }
    /*else if (obj->type->kind == T_ARRAY)
    {
        tp->baseType = obj->type->elemType;
    }*/
    else
    {
        if (obj->level == 0)
            adr = IR_BINOP(IR_i32, IRBK_PLUS, IR_TEMP(IR_i32, "GP"),
                           IR_CONST(IR_i32, obj->adr));
        else
            // FIXME: Add support for local variables in regs A0, A1, A2 and A3.
            adr = IR_BINOP(IR_i32, IRBK_PLUS, IR_TEMP(IR_i32, "SP"),
                           IR_CONST(IR_i32, new FrameOffset(obj->adr)));
    }

    return adr;
}

Type_t IndirectRefASTNode::checkType(AbstractSyntaxTree *ast)
{
    Type_t tp = nullptr;

    if (AST_MATCH_IDENT(_expr))
    {
        Object_t obj;
        SymbolTable *stb = ast->getSymbolTable();
        const char *name = AST_IDENT_VALUE(_expr);
        unsigned nameLineNum = AST_IDENT_LINE_NUM(_expr);

        if (ast->currentRecordObj == nullptr)
            obj = stb->find(name);
        else
            obj = stb->findField(name, ast->currentRecordObj->type->baseType);

        if (obj != stb->noObj)
        {
            if (obj->type->kind != T_POINTER)
                ast->error("object '%s' is not a pointer", name, nameLineNum);

            // Item's type must become same as the base type.
            tp = obj->type->baseType;
            if (!AST_MATCH_NULL(_field))
            {
                // This is a record type being dereferenced.
                // i = p->x;
                // Because p is pointer to an object, object field x must
                // be returned.
                if (AST_MATCH_IDENT(_field))
                {
                    const char *recordField = AST_IDENT_VALUE(_field);
                    unsigned recordFieldLineNum = AST_IDENT_LINE_NUM(_field);
                    Object_t objField = stb->findField(recordField, tp);

                    if (objField != stb->noObj)
                        // Now Item's type must become same as the field's type.
                        tp = objField->type;
                    else
                        ast->error(
                            "object pointed by '%s' does not have field '%s'",
                            obj->name, recordField, recordFieldLineNum);
                }
                else if (AST_MATCH_INDIRECT_REF(_field) ||
                         AST_MATCH_STRUCT_REF(_field))
                {
                    ast->currentRecordObj = obj;
                    tp = _field->checkType(ast);
                }
            }
        }
        else
        {
            ast->error("pointer '%s' does not exist", name, nameLineNum);
        }
    }
    else
    {
        tp = _expr->checkType(ast);
    }

    ast->currentRecordObj = nullptr;

    return tp;
}

/* FIXME:

  Example:

    a->b->c

  This method works for tree form:

      ->
     / \
    ->  c
    / \
   a   b

  Which is incorect, because the form should be:

      ->
     / \
    a  ->
       / \
      b   c

  The tree forms are fixed by introducing the method:
    Parser::parsePostfixExpression()
*/
IRExpr *IndirectRefASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExpr *mem, *x, *mem1, *memOffset;
    Type_t baseTp, tmpTp;
    IRExprTree *iret;
    Object_t obj;
    SymbolTable *stb;
    const char *name;
    unsigned nameLineNum;

    if (!AST_MATCH_IDENT(_expr))
    {
        x = _expr->emitIR(ast);
        mem = IR_MEM(x->getIRType(),
                     IR_BINOP(IR_i32, IRBK_PLUS, IR_CONST(IR_i32, 0), x));
        ast->currentRecordObj = nullptr;
        return mem;
    }

    tmpTp = nullptr;
    stb = ast->getSymbolTable();
    name = AST_IDENT_VALUE(_expr);
    nameLineNum = AST_IDENT_LINE_NUM(_expr);

    if (ast->currentRecordObj == nullptr)
        obj = stb->find(name);
    else
        obj = stb->findField(name, ast->currentRecordObj->type->baseType);

    if (obj == stb->noObj)
        ast->error("pointer '%s' does not exist", name, nameLineNum);

    if (obj->type->kind != T_POINTER)
        ast->error("object '%s' is not a pointer", name, nameLineNum);

    //  int i;
    //  int *p;
    //  ...
    //  i = *p;
    //
    //  In this function p is being dereferenced and value
    //  that p points to must be returned.

    baseTp = obj->type->baseType;

    if (!AST_MATCH_NULL(_field))
    {
        // This is a pointer to record type being dereferenced.
        //  i = p->x;
        //  Because p is pointer to an object, object field x must
        //  be returned.
        if (AST_MATCH_IDENT(_field))
        {
            const char *recordField = AST_IDENT_VALUE(_field);
            unsigned recordFieldLineNum = AST_IDENT_LINE_NUM(_field);
            Object_t objField = stb->findField(recordField, baseTp);

            if (objField == stb->noObj)
                ast->error("object pointed by '%s' does not have field '%s'",
                           obj->name, recordField, recordFieldLineNum);

            // Setup field offset into.
            memOffset = IR_CONST(IR_i32, objField->adr);
            // Now the type must become same as the field's type.
            tmpTp = objField->type;
        }
        else if (AST_MATCH_INDIRECT_REF(_field) || AST_MATCH_STRUCT_REF(_field))
        {
            // FIXME: Make sure that this type is not wrong.
            tmpTp = obj->type->baseType;
            ast->currentRecordObj = obj;
            memOffset = _field->emitIR(ast);
        }
    }
    else
    {
        tmpTp = baseTp;
        memOffset = IR_CONST(IR_i32, 0);
    }

    iret = ast->getIRExprTree();
    mem1 = emitMemoryAccess(obj, ast, lineNum);
    mem = IR_MEM(iret->stbToIRType(tmpTp),
                 IR_BINOP(IR_i32, IRBK_PLUS, memOffset, mem1));

    ast->currentRecordObj = nullptr;

    return mem;
}

Type_t LShiftExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Left shift operator constraints

    stb = ast->getSymbolTable();
    UNLESS(stb->isIntegralType(t1) && stb->isIntegralType(t2))
    ast->error("invalid operands to '<<'", lineNum);

    return t1;
}

IRExpr *LShiftExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret;
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);
    // FIXME: This should be getType(), to not interpret types again.
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Left shift operator constraints

    if (IR_MATCH_CONST(y))
    {
        ConstIRExpr *shiftBy = static_cast<ConstIRExpr *>(y);
        int typeSize = t2->size * 8;

        if (shiftBy->getValue() >= typeSize)
        {
            if (shiftBy->getValue() == typeSize)
                ast->error("left shift count is equal to width of type",
                           lineNum);
            else
                ast->error("left shift count is greater then width of type",
                           lineNum);
        }
    }

    iret = ast->getIRExprTree();
    return IR_BINOP(iret->stbToIRType(tp), IRBK_LSHIFT, x, y);
}

Type_t RShiftExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Right shift operator constraints

    stb = ast->getSymbolTable();
    UNLESS(stb->isIntegralType(t1) && stb->isIntegralType(t2))
    ast->error("invalid operands to '>>'", lineNum);

    return t1;
}

IRExpr *RShiftExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret;
    IRBinopKind binopKind;
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);
    // FIXME: This should be getType(), to not interpret types again.
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Right shift operator constraints

    if (IR_MATCH_CONST(y))
    {
        ConstIRExpr *shiftBy = static_cast<ConstIRExpr *>(y);
        int typeSize = t2->size * 8;

        if (shiftBy->getValue() >= typeSize)
        {
            if (shiftBy->getValue() == typeSize)
                ast->error("right shift count is equal to width of type",
                           lineNum);
            else
                ast->error("right shift count is greater then width of type",
                           lineNum);
        }
    }

    iret = ast->getIRExprTree();
    binopKind = t1->isSigned ? IRBK_ARSHIFT : IRBK_RSHIFT;
    return IR_BINOP(iret->stbToIRType(tp), binopKind, x, y);
}

Type_t BitIorExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Bitwise inclusive OR operator constraints

    stb = ast->getSymbolTable();
    UNLESS(stb->isIntegralType(t1) && stb->isIntegralType(t2))
    ast->error("invalid operands to '|'", lineNum);

    return t1;
}

IRExpr *BitIorExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);

    return IR_BINOP(iret->stbToIRType(tp), IRBK_OR, x, y);
}

Type_t BitXorExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Bitwise exclusive OR operator constraints

    stb = ast->getSymbolTable();
    UNLESS(stb->isIntegralType(t1) && stb->isIntegralType(t2))
    ast->error("invalid operands to '^'", lineNum);

    return t1;
}

IRExpr *BitXorExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);

    return IR_BINOP(iret->stbToIRType(tp), IRBK_XOR, x, y);
}

Type_t BitAndExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Bitwise AND operator constraints

    stb = ast->getSymbolTable();
    UNLESS(stb->isIntegralType(t1) && stb->isIntegralType(t2))
    ast->error("invalid operands to '&'", lineNum);

    return t1;
}

IRExpr *BitAndExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);

    return IR_BINOP(iret->stbToIRType(tp), IRBK_AND, x, y);
}

Type_t LogAndExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Logical AND operator constraints

    stb = ast->getSymbolTable();
    UNLESS(stb->isScalarType(t1) && stb->isScalarType(t2))
    ast->error("invalid operands to '&&'", lineNum);

    return t1;
}

// Logical AND algorithm:
//
//   PUSH true_lab
//   TRAVERSE(LHS)
//   CJUMP F prev_false_lab
//   DEFINE true_lab
//   POP true_lab
//   TRAVERSE(RHS)
//   CJUMP F prev_false_lab
//   CJUMP T prev_true_lab

IRExpr *LogAndExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    SeqIRExpr *seq = IR_SEQ();
    char true_lab[LABEL_SIZE];
    const char *prev_true_lab = ast->getTopTrueLabel();
    const char *prev_false_lab = ast->getTopFalseLabel();
    unsigned labc = iret->getLabelCount();
    IRExpr *x, *y;

    checkType(ast);

    snprintf(true_lab, LABEL_SIZE, "true_lab_%d", labc);
    ast->pushTrueLabel(true_lab);

    x = _lhs->emitIR(ast);

    if (IR_MATCH_RELOP(x))
    {
        seq->add(IR_CJUMP(x, NULL_IR_EXPR, IR_NAME(prev_false_lab)));
    }
    else if (IR_MATCH_SEQ(x))
    {
        seq->add(x);
    }
    else
    {
        seq->add(IR_CJUMP(IR_RELOP(IRRK_NE, x, IR_CONST(IR_i32, 0)),
                          NULL_IR_EXPR, IR_NAME(prev_false_lab)));
    }

    seq->add(IR_LABEL(true_lab));
    ast->popTrueLabel();

    y = _rhs->emitIR(ast);
    if (IR_MATCH_RELOP(y))
    {
        seq->add(IR_CJUMP(y, IR_NAME(prev_true_lab), IR_NAME(prev_false_lab)));
    }
    else if (IR_MATCH_SEQ(y))
    {
        seq->add(y);
    }
    else
    {
        seq->add(IR_CJUMP(IR_RELOP(IRRK_NE, y, IR_CONST(IR_i32, 0)),
                          IR_NAME(prev_true_lab), IR_NAME(prev_false_lab)));
    }

    // Logical AND operator type constraints

    // assert(x->type != NULL && y->type != NULL);

    // Type_t t1 = x->type;
    // Type_t t2 = y->type;

    // UNLESS(stb->isScalarType(t1) && stb->isScalarType(t2))
    //  ast->error("invalid operands to '&&'", lineNum);

    return seq;
}

Type_t LogOrExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Logical OR operator constraints

    stb = ast->getSymbolTable();
    UNLESS(stb->isScalarType(t1) && stb->isScalarType(t2))
    ast->error("invalid operands to '||'", lineNum);

    return t1;
}

// Logical OR algorithm:
//
//   PUSH false_lab
//   TRAVERSE(LHS)
//   CJUMP T prev_true_lab
//   DEFINE false_lab
//   POP false_lab
//   TRAVERSE(RHS)
//   CJUMP T prev_true_lab
//   CJUMP F prev_false_lab

IRExpr *LogOrExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    SeqIRExpr *seq = IR_SEQ();
    IRExpr *x, *y;
    char false_lab[LABEL_SIZE];
    const char *prev_true_lab = ast->getTopTrueLabel();
    const char *prev_false_lab = ast->getTopFalseLabel();
    IRExprTree *iret = ast->getIRExprTree();
    unsigned labc = iret->getLabelCount();

    checkType(ast);

    snprintf(false_lab, LABEL_SIZE, "false_lab_%d", labc);
    ast->pushFalseLabel(false_lab);

    x = _lhs->emitIR(ast);
    if (IR_MATCH_RELOP(x))
        seq->add(IR_CJUMP(x, IR_NAME(prev_true_lab), NULL_IR_EXPR));
    else if (IR_MATCH_SEQ(x))
        seq->add(x);
    else
        seq->add(IR_CJUMP(IR_RELOP(IRRK_NE, x, IR_CONST(IR_i32, 0)),
                          IR_NAME(prev_true_lab), NULL_IR_EXPR));

    seq->add(IR_LABEL(false_lab));
    ast->popFalseLabel();

    y = _rhs->emitIR(ast);

    if (IR_MATCH_RELOP(y))
        seq->add(IR_CJUMP(y, IR_NAME(prev_true_lab), IR_NAME(prev_false_lab)));
    else if (IR_MATCH_SEQ(y))
        seq->add(y);
    else
        seq->add(IR_CJUMP(IR_RELOP(IRRK_NE, y, IR_CONST(IR_i32, 0)),
                          IR_NAME(prev_true_lab), IR_NAME(prev_false_lab)));

    // Logical OR operator type constraints

    // assert(x->type != NULL && y->type != NULL);

    // Type_t t1 = x->type;
    // Type_t t2 = y->type;

    // UNLESS(stb->isScalarType(t1) && stb->isScalarType(t2))
    //  ast->error("invalid operands to '&&'", lineNum);

    return seq;
}

Type_t PlusExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Addition constraints

    stb = ast->getSymbolTable();
    UNLESS((stb->isArithmeticType(t1) && stb->isArithmeticType(t2)) ||
           (stb->isIntegralType(t1) && t2->kind == T_POINTER) ||
           (stb->isIntegralType(t2) && t1->kind == T_POINTER))
    ast->error("invalid operands to binary '+'", lineNum);

    if (t2->kind == T_POINTER)
        return t2;

    return t1;
}

IRExpr *PlusExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);

    // TODO: Implement type conversions.
    // if (stb->isIntegralType(t1) && stb->isRealType(t2)) {
    // Convert the of x to type of y.
    // gen->convert(x, y);
    //}

    // FIXME: This is temporary solution.
    // At this point both types of x and y should be the same.
    // if (stb->isRealType(x->type) && stb->isRealType(y->type))
    //  gen->binaryOpFP(GEN_ADD, x, y);
    // else
    //  gen->binaryOp(GEN_ADD, x, y);

    return IR_BINOP(iret->stbToIRType(tp), IRBK_PLUS, x, y);
}

Type_t MinusExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb = ast->getSymbolTable();
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Subtraction constraints

    UNLESS((stb->isArithmeticType(t1) && stb->isArithmeticType(t2)) ||
           (t1->kind == T_POINTER && stb->isIntegralType(t2)) ||
           (stb->equalBasePointerTypes(t1, t2)))
    ast->error("invalid operands to binary '-'", lineNum);

    return t1;
}

IRExpr *MinusExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);

    return IR_BINOP(iret->stbToIRType(tp), IRBK_MINUS, x, y);
}

Type_t MultExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb = ast->getSymbolTable();
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Multiplication constraints

    UNLESS(stb->isArithmeticType(t1) && stb->isArithmeticType(t2))
    ast->error("invalid operands to binary '*'", lineNum);

    return t1;
}

IRExpr *MultExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);

    return IR_BINOP(iret->stbToIRType(tp), IRBK_MUL, x, y);
}

Type_t TruncDivExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb = ast->getSymbolTable();
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Division constraints

    UNLESS(stb->isArithmeticType(t1) && stb->isArithmeticType(t2))
    ast->error("invalid operands to '/'", lineNum);

    return t1;
}

IRExpr *TruncDivExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);

    return IR_BINOP(iret->stbToIRType(tp), IRBK_DIV, x, y);
}

Type_t TruncModExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Modulo constraints

    stb = ast->getSymbolTable();
    UNLESS(stb->isIntegralType(t1) && stb->isIntegralType(t2))
    ast->error("invalid operands to '%'", lineNum);

    return t1;
}

IRExpr *TruncModExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);

    return IR_BINOP(iret->stbToIRType(tp), IRBK_MOD, x, y);
}

Type_t ArrayRefASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t indexType;
    Type_t exprType = _expr->checkType(ast);

    if (exprType->kind != T_ARRAY && exprType->kind != T_POINTER)
        ast->error("object is not an array", lineNum);

    indexType = _index->checkType(ast);
    stb = ast->getSymbolTable();
    UNLESS(stb->isIntegralType(indexType))
    ast->error("array subscript is not an integer", lineNum);

    if (exprType->kind == T_ARRAY)
        return exprType->elemType;

    return exprType->baseType;
}

// Pattern for generating pointers:
//   ArrayRef:
//   (mem
//     (binop plus
//            (mem           <-- array base
//              (binop plus
//                     (const k)
//                     (temp sp)))
//            (const n)))    <-- array local offset

IRExpr *ArrayRefASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret;
    SymbolTable *stb;
    IRExpr *e, *idx;
    Object_t obj;
    Type_t tp = checkType(ast);

    if (tp == nullptr)
        return NULL_IR_EXPR;

    e = _expr->emitIR(ast);
    idx = _index->emitIR(ast);

    assert(AST_MATCH_IDENT(_expr));

    stb = ast->getSymbolTable();
    obj = stb->find(AST_IDENT_VALUE(_expr));

    // Create indirect reference for array declarations such as int *a or int
    // a[].
    // The point is to wrap e with the pattern:
    // (mem (binop plus
    //             (const 0)
    //             e)
    // so it will be matched by IRExprTree:matchIndirectRef().

    if (obj != stb->noObj && obj->type->kind == T_POINTER)
        e = IR_MEM(IR_i32, IR_BINOP(IR_i32, IRBK_PLUS, IR_CONST(IR_i32, 0), e));

    iret = ast->getIRExprTree();
    return IR_MEM(
        iret->stbToIRType(tp),
        IR_BINOP(IR_i32, IRBK_PLUS, e,
                 IR_BINOP(IR_i32, IRBK_MUL, idx, IR_CONST(IR_i32, tp->size))));
}

Type_t StructRefASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb = ast->getSymbolTable();
    Type_t tp = nullptr;
    const char *recordName = AST_IDENT_VALUE(_name);
    unsigned recordLineNum = AST_IDENT_LINE_NUM(_name);

    if (ast->currentRecordObj == nullptr)
    {
        ast->currentRecordObj = stb->find(recordName);
        if (ast->currentRecordObj == stb->noObj)
            ast->error("struct '%s' does not exist", recordName, recordLineNum);
    }
    else
    {
        // Record in record access i.e.  -->  x.y.z;
        Object_t tmpObj =
            stb->findField(recordName, ast->currentRecordObj->type);

        if (tmpObj == stb->noObj)
            ast->error("struct '%s' does not have a field '%s'",
                       ast->currentRecordObj->name, recordName, recordLineNum);
        ast->currentRecordObj = tmpObj;
    }

    if (AST_MATCH_IDENT(_member))
    {
        const char *memberName = AST_IDENT_VALUE(_member);
        unsigned memberLineNum = AST_IDENT_LINE_NUM(_member);
        Object_t obj = stb->findField(memberName, ast->currentRecordObj->type);

        if (obj != stb->noObj)
            tp = obj->type;
        else
            ast->error("struct '%s' does not have a field '%s'", recordName,
                       memberName, memberLineNum);
    }
    else if (AST_MATCH_STRUCT_REF(_member) || AST_MATCH_INDIRECT_REF(_member))
    {
        tp = _member->checkType(ast);
    }

    ast->currentRecordObj = nullptr;

    return tp;
}

IRExpr *StructRefASTNode::emitIR(AbstractSyntaxTree *ast)
{
    SymbolTable *stb = ast->getSymbolTable();
    IRExpr *rec = NULL_IR_EXPR, *mbr = NULL_IR_EXPR;
    Object_t obj;
    const char *recordName = AST_IDENT_VALUE(_name);
    unsigned recordLineNum = AST_IDENT_LINE_NUM(_name);

    if (ast->currentRecordObj == nullptr)
    {
        ast->currentRecordObj = stb->find(recordName);
        if (ast->currentRecordObj == stb->noObj)
            ast->error("struct '%s' does not exist", recordName, recordLineNum);
    }
    else
    {
        Object_t tmpObj;

        if (ast->currentRecordObj->type->kind != T_STRUCT)
            ast->error("identifier '%s' is not a struct", recordName,
                       recordLineNum);

        // Record in record access i.e.  -->  x.y.z;
        tmpObj = stb->findField(recordName, ast->currentRecordObj->type);

        if (tmpObj == stb->noObj)
            ast->error("struct '%s' does not have a field '%s'",
                       ast->currentRecordObj->name, recordName, recordLineNum);
        ast->currentRecordObj = tmpObj;
    }
    obj = ast->currentRecordObj;
    rec = emitMemoryAccess(obj, ast, lineNum);

    if (AST_MATCH_IDENT(_member))
    {
        const char *memberName = AST_IDENT_VALUE(_member);
        unsigned memberLineNum = AST_IDENT_LINE_NUM(_member);
        Object_t obj = stb->findField(memberName, ast->currentRecordObj->type);

        if (obj != stb->noObj)
        {
            mbr = IR_CONST(IR_i32, obj->adr);
            mbr->setType(obj->type);
        }
        else
        {
            ast->error("struct '%s' does not have a field '%s'", recordName,
                       memberName, memberLineNum);
        }
    }
    else if (AST_MATCH_STRUCT_REF(_member) || AST_MATCH_INDIRECT_REF(_member))
    {
        mbr = _member->emitIR(ast);
        mbr->setType(stb->intType);
    }
    ast->currentRecordObj = nullptr;

    return IR_MEM(IR_i32, IR_BINOP(IR_i32, IRBK_PLUS, rec, mbr));
}

Type_t LtExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    // Relational constraints

    assert(t1 != nullptr && t2 != nullptr);

    stb = ast->getSymbolTable();
    UNLESS((stb->isArithmeticType(t1) && stb->isArithmeticType(t2)) ||
           (stb->equalBasePointerTypes(t1, t2)))
    ast->error("invalid operands to '<'", lineNum);

    // FIXME: This is not correct way to determine the type.

    return t1;
}

IRExpr *LtExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);

    return new RelopIRExpr(iret->stbToIRType(tp), IRRK_LT, x, y);
}

Type_t LeExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    // Relational constraints

    assert(t1 != nullptr && t2 != nullptr);

    stb = ast->getSymbolTable();
    UNLESS((stb->isArithmeticType(t1) && stb->isArithmeticType(t2)) ||
           (stb->equalBasePointerTypes(t1, t2)))
    ast->error("invalid operands to '<='", lineNum);

    // FIXME: This is not correct way to determine the type.

    return t1;
}

IRExpr *LeExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);

    return new RelopIRExpr(iret->stbToIRType(tp), IRRK_LE, x, y);
}

Type_t GtExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    // Relational constraints

    assert(t1 != nullptr && t2 != nullptr);

    stb = ast->getSymbolTable();
    UNLESS((stb->isArithmeticType(t1) && stb->isArithmeticType(t2)) ||
           (stb->equalBasePointerTypes(t1, t2)))
    ast->error("invalid operands to '>'", lineNum);

    // FIXME: This is not correct way to determine the type.

    return t1;
}

IRExpr *GtExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);

    return new RelopIRExpr(iret->stbToIRType(tp), IRRK_GT, x, y);
}

Type_t GeExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    // Relational constraints

    assert(t1 != nullptr && t2 != nullptr);

    stb = ast->getSymbolTable();
    UNLESS((stb->isArithmeticType(t1) && stb->isArithmeticType(t2)) ||
           (stb->equalBasePointerTypes(t1, t2)))
    ast->error("invalid operands to '>='", lineNum);

    // FIXME: This is not correct way to determine the type.

    return t1;
}

IRExpr *GeExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);

    return new RelopIRExpr(iret->stbToIRType(tp), IRRK_GE, x, y);
}

Type_t EqExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Equality operator constraints

    stb = ast->getSymbolTable();
    UNLESS((stb->isArithmeticType(t1) && stb->isArithmeticType(t2)) ||
           (t1->kind == T_POINTER && t2 == stb->nullType) ||
           (t2->kind == T_POINTER && t1 == stb->nullType) ||
           (stb->equalBasePointerTypes(t1, t2)))
    ast->error("invalid operands to '=='", lineNum);

    return t1;
}

IRExpr *EqExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);

    return new RelopIRExpr(iret->stbToIRType(tp), IRRK_EQ, x, y);
}

Type_t NeExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb;
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    assert(t1 != nullptr && t2 != nullptr);

    // Equality operator constraints

    stb = ast->getSymbolTable();
    UNLESS((stb->isArithmeticType(t1) && stb->isArithmeticType(t2)) ||
           (t1->kind == T_POINTER && t2 == stb->nullType) ||
           (t2->kind == T_POINTER && t1 == stb->nullType) ||
           (stb->equalBasePointerTypes(t1, t2)))
    ast->error("invalid operands to '!='", lineNum);

    return t1;
}

IRExpr *NeExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    Type_t tp = checkType(ast);
    IRExpr *x = _lhs->emitIR(ast);
    IRExpr *y = _rhs->emitIR(ast);

    return new RelopIRExpr(iret->stbToIRType(tp), IRRK_NE, x, y);
}

static bool isLValue(IRExpr *x)
{
    return (IR_MATCH_MEM(x) || IR_MATCH_TEMP(x));
}

Type_t AssignExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb = ast->getSymbolTable();
    Type_t t1 = _lhs->checkType(ast);
    Type_t t2 = _rhs->checkType(ast);

    // Assignment constraints

    // TODO: Implement more precise type checking for function pointers.
    UNLESS((stb->isArithmeticType(t1) && stb->isArithmeticType(t2)) ||
           (t1->kind == T_POINTER && t2->baseType == stb->noType) ||
           (t2->kind == T_POINTER && t1->baseType == stb->noType) ||
           (t1->kind == T_POINTER && t2 == stb->nullType) ||
           (t1->kind == T_POINTER && t2->kind == T_ARRAY) ||
           (t2->kind == T_POINTER && t1->kind == T_ARRAY) ||
           (stb->equalFunctionPointerTypes(t1, t2)) ||
           (stb->equalBasePointerTypes(t1, t2)))
    ast->error("incompatible types of assignment", lineNum);

    if ((t1->kind == T_POINTER && t2->kind == T_ARRAY &&
         t1->baseType->kind != t2->elemType->kind) ||
        (t2->kind == T_POINTER && t1->kind == T_ARRAY &&
         t2->baseType->kind != t1->elemType->kind))
        ast->warning("assignment from incompatible pointer type", lineNum);

    return t1;
}

static bool isReadOnly(const char *name, SymbolTable *stb)
{
    Object_t obj = stb->find(name);

    if (obj != stb->noObj && obj->isConstant)
        return true;

    return false;
}

/*
  FIXME:

    For the following C code:

      char str[10];
      char *p;
      p = str

    The following IR subtree IS generated:

      (move
        (mem i32
          (binop plus i32
                 (temp SP)
                 (const i32 0)))
        (mem i32
          (binop plus i32
                 (temp SP)
                 (const i32 0))))

    but the follwing IR subtree SHOULD BE generated:

      (move
        (mem i32
          (binop plus i32
                 (temp SP)
                 (const i32 0)))
        (binop plus i32
               (temp SP)
               (const i32 0)))
*/
IRExpr *AssignExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    SymbolTable *stb = ast->getSymbolTable();
    IRExpr *x, *y;

    checkType(ast);

    // An object is read-only if it has 'const' qualifier in declaration.
    if (AST_MATCH_IDENT(_lhs))
    {
        const char *name = AST_IDENT_VALUE(_lhs);

        if (isReadOnly(name, stb))
            ast->error("assignment of read-only variable '%s'", name, lineNum);
    }
    else if (AST_MATCH_ARRAY_REF(_lhs))
    {
        if (AST_MATCH_IDENT(AST_ARRAY_REF_EXPR(_lhs)))
        {
            const char *name = AST_IDENT_VALUE(_lhs);

            if (isReadOnly(name, stb))
                ast->error("assignment of read-only location '%s'", name,
                           lineNum);
        }
    }
    else if (AST_MATCH_STRUCT_REF(_lhs) || AST_MATCH_INDIRECT_REF(_lhs))
    {
        // TODO: Check if struct or indirect ref are not read-only.
    }

    x = _lhs->emitIR(ast);

    // LValue check
    if (!isLValue(x))
        ast->error("lvalue required as left operand of assignment", lineNum);

    y = _rhs->emitIR(ast);

    // (move x y)
    // TODO: Shouldn't this be:
    //return IR_MOVE(iret->stbToIRType(type), x, y);

    return IR_MOVE(y->getIRType(), x, y);
}

Type_t CallExprASTNode::checkType(AbstractSyntaxTree *ast)
{
    SymbolTable *stb = ast->getSymbolTable();
    const char *funcName = AST_IDENT_VALUE(_expr);
    unsigned funcLineNum = AST_IDENT_LINE_NUM(_expr);
    Object_t func = stb->find(funcName);

    if (func == stb->noObj)
        ast->error("function '%s' does not exist", funcName, funcLineNum);

    // Get the number of actual arguments.
    if (AST_MATCH_LIST(_args))
    {
        std::vector<ASTNode *> &argVec = AST_LIST_ELEMENTS(_args);

        // Check if number of actual and formal arguments is the same.
        // FIXME: This is temporary.
        if (stb->isFunctionPointerType(func->type))
        {
            // TODO: Check number of arguments
        }
        else if (func->prmc != argVec.size())
        {
            ast->error("invalid number of arguments for function '%s'",
                       funcName, funcLineNum);
        }

        // TODO: Check types of arguments (if any).

        // Load arguments onto the function stack.
        for (unsigned i = 0; i < argVec.size(); i++)
        {
            // TODO: Check if types match for the formal and the actual
            // arguments.
            Type_t argType = argVec[i]->checkType(ast);
        }
    }

    if (stb->isFunctionPointerType(func->type))
        // Return type of function pointed by a pointer.
        return func->type->baseType->funcType;
    else
        return func->type;
}

IRExpr *CallExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExprTree *iret = ast->getIRExprTree();
    std::vector<IRExpr *> callArgs;
    SymbolTable *stb = ast->getSymbolTable();
    const char *funcName = AST_IDENT_VALUE(_expr);
    unsigned funcLineNum = AST_IDENT_LINE_NUM(_expr);
    Object_t func = stb->find(funcName);
    CallIRExpr *callExpr;

    if (func == stb->noObj)
        ast->error("function '%s' does not exist", funcName, funcLineNum);

    // Get the number of actual arguments.
    if (AST_MATCH_LIST(_args))
    {
        std::vector<ASTNode *> &argVec = AST_LIST_ELEMENTS(_args);

        // Check if number of actual and formal arguments is the same.
        // FIXME: This is temporary.
        if (stb->isFunctionPointerType(func->type))
        {
            // TODO: Check number of arguments
        }
        else if (func->prmc != argVec.size())
        {
            ast->error("invalid number of arguments for function '%s'",
                       funcName, funcLineNum);
        }

        // Load arguments onto the function stack.
        for (unsigned i = 0; i < argVec.size(); i++)
        {
            IRExpr *arg;
            Object_t obj;
            CodeGenerator *gen;

            if (!AST_MATCH_IDENT(argVec[i]))
            {
                // callArgs.push_back(argVec[i]->emitIR(ast));
                arg = argVec[i]->emitIR(ast);
                arg->setSize(4);
                callArgs.push_back(arg);
                continue;
            }

            // TODO: Check if types match for the formal and the actual
            // arguments.
            obj = stb->find(AST_IDENT_VALUE(argVec[i]));

            // Various cases must be handled for various types of objects. If an
            // object is an array pointer the address of the object must be
            // passed to the callee, i.e. SP + FrameOffset(obj->adr). If object
            // is struct, all the object's bytes must be passed to callee
            // function i.e. the bytes must be copied to its stack.
            // If GNU ABI is used the registers A0-A3 are used to pass first 16
            // bytes to callee.

            // TODO: Double check this for the first 16 bytes.

            if (obj->type->kind != T_ARRAY)
            {
                // callArgs.push_back(argVec[i]->emitIR(ast));
                arg = argVec[i]->emitIR(ast);
                arg->setSize(obj->type->size);
                callArgs.push_back(arg);
                continue;
            }

            gen = ast->getCodeGenerator();

            if (obj->level == 0)
            {
                // Passing address of an array: GP + obj->adr.
                //
                // TODO: Shouldn't this be done as AddrExpr? Or that is to say
                // AddrExpr to be created by parser for array variables?

                arg = IR_BINOP(IR_i32, IRBK_PLUS, IR_TEMP(IR_i32, "GP"),
                               IR_CONST(IR_i32, obj->adr));
            }
            else
            {
                // Passing address of an array: SP + obj->adr.
                //   TODO: Same as for the GP case, shouldn't this be done as
                // AddrExpr? Or that is to say AddrExpr to be created by parser
                // for array variables?
#ifdef GNU_ABI
                // Add support to get values of local variables from regs A0,
                // A1, A2 and A3.
                // According to GNU ABI first four arguments are copied to the
                // registers A0-A3. Since the object is an array, the address of
                // the array is in one of these registers if it is between first
                // four arguments of the caller.
                // The values for callee are copied to A0-A3 in the function
                // [Arch]CodeGenerator::push().

                if (obj->kind == OBJ_PAR && obj->parIndex < 4)
                {
                    const char *reg = nullptr;

                    switch (obj->parIndex)
                    {
                        case 0:
                            reg = "A0";
                            break;
                        case 1:
                            reg = "A1";
                            break;
                        case 2:
                            reg = "A2";
                            break;
                        case 3:
                            reg = "A3";
                            break;
                    }
                    arg = IR_TEMP(IR_i32, reg);
                }
                else
                {
                    arg = IR_BINOP(IR_i32, IRBK_PLUS, IR_TEMP(IR_i32, "SP"),
                                   IR_CONST(IR_i32, new FrameOffset(obj->adr)));
                }
#else
                arg = IR_BINOP(IR_i32, IRBK_PLUS, IR_TEMP(IR_i32, "SP"),
                               IR_CONST(IR_i32, new FrameOffset(obj->adr)));
#endif
            }
            arg->setSize(gen->getPointerSize());
            callArgs.push_back(arg);
        }
    }

    if (stb->isFunctionPointerType(func->type))
    {
        IRExpr *funcVar = _expr->emitIR(ast);

        // Set type to the return type of function pointed by a pointer.

        callExpr = IR_CALL(funcVar, callArgs);
        callExpr->setIRType(iret->stbToIRType(func->type->baseType->funcType));
    }
    else
    {
        callExpr = IR_CALL(IR_NAME(funcName), callArgs);
        callExpr->setIRType(iret->stbToIRType(func->type));
    }

    return callExpr;
}

IRExpr *PutCharExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    std::vector<IRExpr *> callArgs;

    callArgs.push_back(_expr->emitIR(ast));
    return IR_CALL(IR_NAME("putc"), callArgs);
}

IRExpr *PutIntExprASTNode::emitIR(AbstractSyntaxTree *ast)
{
    std::vector<IRExpr *> callArgs;

    callArgs.push_back(_expr->emitIR(ast));
    return IR_CALL(IR_NAME("puti"), callArgs);
}

IRExpr *ListASTNode::emitIR(AbstractSyntaxTree *ast)
{
    SeqIRExpr *seq = IR_SEQ();

    for (unsigned i = 0; i < _elements.size(); i++)
        if (_elements[i] != NULL_AST_NODE)
            seq->add(_elements[i]->emitIR(ast));

    return seq;
}

IRExpr *VarDeclASTNode::emitIR(AbstractSyntaxTree *ast)
{
    IRExpr *res, *x, *y;
    Type_t tp;

    if (AST_MATCH_NULL(_name) || AST_MATCH_NULL(_init))
        return NULL_IR_EXPR;

    res = NULL_IR_EXPR;
    // tp = checkType(ast);
    x = _name->emitIR(ast);

    // LValue check
    if (!isLValue(x))
        ast->error("lvalue required as left operand of assignment", lineNum);

    if (AST_MATCH_LIST(_init))
    {
        std::vector<ASTNode *> &initVec = AST_LIST_ELEMENTS(_init);
        SeqIRExpr *seq = IR_SEQ();
        // IRExprTree *iret = ast->getIRExprTree();

        // TODO: Check if var is variable is array.

        for (unsigned i = 0; i < initVec.size(); i++)
        {
            // FIXME:
            //IRType irtype = iret->stbToIRType(tp);
            //unsigned irtypeSize = tp->size;

            IRExpr *idx, *arrayRef;
            IRType irtype = IR_i32;
            unsigned irtypeSize = 4;

            idx = IR_CONST(IR_i32, i);
            arrayRef = IR_MEM(irtype,
                              IR_BINOP(IR_i32, IRBK_PLUS, x,
                                       IR_BINOP(IR_i32, IRBK_MUL, idx,
                                                IR_CONST(IR_i32, irtypeSize))));

            y = initVec[i]->emitIR(ast);
            seq->add(IR_MOVE(IR_i32, arrayRef, y));
        }

        res = seq;
    }
    else
    {
        y = _init->emitIR(ast);

        // (move x y)
        res = IR_MOVE(IR_i32, x, y);
    }

    return res;
}

//
// Declare methods
//

void TypeDeclASTNode::declare(AbstractSyntaxTree *ast)
{
    SymbolTable *stb = ast->getSymbolTable();

    // FIXME: For pointer types baseType need to be declared as well.

    stb->insert(AST_IDENT_VALUE(_name), OBJ_TYPE, ast->getStbType(_body));
}

void FunctionDeclASTNode::declare(AbstractSyntaxTree *ast)
{
    assert(_name != NULL_AST_NODE && "Function declaration name is NULL.");

    SymbolTable *stb = ast->getSymbolTable();
    unsigned funcPrmc = 0;
    Type_t funcType = ast->getStbType(_type);
    const char *fnName = AST_IDENT_VALUE(_name);
    Object_t obj = stb->find(fnName);

    if (AST_MATCH_LIST(_prms))
    {
        std::vector<ASTNode *> &tmpVec = AST_LIST_ELEMENTS(_prms);
        funcPrmc = tmpVec.size();
    }

    if (obj == stb->noObj)
    {
        CodeGenerator *gen = ast->getCodeGenerator();

        obj = stb->insert(fnName, OBJ_FUNC, funcType);
        obj->adr = -1;
        obj->prmc = funcPrmc;
        gen->addSymbol(fnName, -1);
    }
    else
    {
        // Check if number of parameters match.
        if (obj->prmc != funcPrmc)
            ast->error("parameter count does not match for '%s'", fnName,
                       _name->getLineNum());

        if (obj->type != funcType)
            ast->error("conflicting types for '%s'", fnName,
                       _name->getLineNum());
    }
    ast->setCurrentFuncDecl(obj);
}

void VarDeclASTNode::declare(AbstractSyntaxTree *ast)
{
    assert(_name != NULL_AST_NODE && "invalid variable declaration");

    // FIXME: Move setting flags from type to decl!!!
    unsigned flags = _type->getFlags();
    SymbolTable *stb = ast->getSymbolTable();
    CodeGenerator *gen = ast->getCodeGenerator();
    Type_t varType = ast->getStbType(_type);
    const char *varName = AST_IDENT_VALUE(_name);
    unsigned varLineNum = AST_IDENT_LINE_NUM(_name);
    Object_t obj = nullptr;

    // Initialization of non-static/local variables is performed in
    // VarDeclASTNode::emitIR().

    if (stb->getLevel() == 0 && !AST_MATCH_NULL(_init) && !isConstant(_init))
        ast->error("initializer element of '%s' is not a constant", varName,
                   varLineNum);

    setFlags(flags);
    if ((SCS_STATIC & flags) != 0x0)
    {
        if (stb->getLevel() == 0)
            obj = stb->insertGlobalVariable(varName, varType, nullptr);
        else
            obj = stb->insertGlobalVariable(varName, varType,
                                            ast->getCurrentFuncDecl()->name);

        // TODO: Add initializer constant to data section.
        // Other constant kinds are not suported so far.
        if (AST_MATCH_INTEGER_CONST(_init))
        {
            int value = AST_INTEGER_CONST_VALUE(_init);

            if (obj != stb->noObj)
                gen->addToDataSection(obj->adr, value);
        }
    }
    else
    {
        obj = stb->insert(varName, OBJ_VAR, varType);
    }

    if (obj == stb->noObj)
    {
        ast->error("identifier '%s' already exists", varName, varLineNum);
    }
    else
    {
        if ((SCS_REGISTER & flags) != 0x0)
        {
            obj->inRegister = true;
            obj->reg = gen->getNonTmpReg();
        }

        // FIXME: This doesn't work for an array declaration, because the flags
        // are lost.
        if ((TQ_CONST & flags) != 0x0)
        {
            obj->isConstant = true;
        }
    }
}

void ParmDeclASTNode::declare(AbstractSyntaxTree *ast)
{
    assert(_name != NULL_AST_NODE && "invalid parameter declaration");

    SymbolTable *stb = ast->getSymbolTable();
    Type_t parmType = ast->getStbType(_type);
    const char *parmName = AST_IDENT_VALUE(_name);
    Object_t obj = stb->insert(parmName, OBJ_PAR, parmType);

    if (obj == stb->noObj)
        ast->error("identifier '%s' already exists", parmName,
                   AST_IDENT_LINE_NUM(_name));
}

void FieldDeclASTNode::declare(AbstractSyntaxTree *ast)
{
    assert(_name != NULL_AST_NODE && "invalid field declaration");

    SymbolTable *stb = ast->getSymbolTable();
    Type_t fieldType = ast->getStbType(_type);

    stb->insert(AST_IDENT_VALUE(_name), OBJ_VAR, fieldType);
}

void ListASTNode::declare(AbstractSyntaxTree *ast)
{
    for (unsigned i = 0; i < _elements.size(); i++)
        _elements[i]->declare(ast);
}
