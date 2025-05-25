// PICCO parser - implementation file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#include "../include/Parser.h"
#include "../include/AbstractSyntaxTree.h"
#include "../include/Lexer.h"
#include <cassert>
#include <stack>
#include <iostream>

void Parser::getTok()
{
    _tokIdx = _laIdx;
    _laIdx = (_laIdx + 1) % TOK_BUF_LEN;
    _newLaIdx = (_laIdx + 2) % TOK_BUF_LEN;

    // Get the current token
    _tok = &_tokbuf[_tokIdx];

    // Get symbol type of first lookahead token
    _sym = _tokbuf[_laIdx].kind;

    // Allocate a new third lookahead token
    _lex->next(&_tokbuf[_newLaIdx]);
}

TokenKind Parser::getLATok(unsigned k)
{
    assert((k < TOK_BUF_LEN) &&
           "k is greater than number of possible lookahead tokens");

    return _tokbuf[(_tokIdx + k) % TOK_BUF_LEN].kind;
}

const char *Parser::getLATokInfoSval(unsigned k)
{
    assert((k < TOK_BUF_LEN) &&
           "k is greater than number of possible lookahead tokens");

    return _tokbuf[(_tokIdx + k) % TOK_BUF_LEN].info.sval;
}

void Parser::parsingError(const char *msg)
{
    std::cout << "line: " << _tok->line << "; col: " << _tok->col << "; error: "
              << msg << std::endl;
    _parsingErrors++;
}

void Parser::check(TokenKind expected)
{
    if (_sym == expected)
    {
        getTok();
    }
    else
    {
        char msg[MAXSTR];

        snprintf(msg, MAXSTR, "expected '%s'", _name[expected]);
        parsingError(msg);
    }
}

void Parser::initTokenBuffer()
{
    _sym = _lex->next(&_tokbuf[0]);
    _lex->next(&_tokbuf[1]);
    _lex->next(&_tokbuf[2]);
    _laIdx = 0;
}

bool CParser::isTypeSpecifier(TokenKind kind, int n)
{
    if (kind == TK_CHAR || kind == TK_DOUBLE || kind == TK_ENUM ||
        kind == TK_FLOAT || kind == TK_INT || kind == TK_LONG ||
        kind == TK_SHORT || kind == TK_SIGNED || kind == TK_STRUCT ||
        kind == TK_UNION || kind == TK_UNSIGNED || kind == TK_VOID)
    {
        return true;
    }
    else if (kind == TK_IDENT)
    {
        Object_t obj = _stb.find(getLATokInfoSval(n));
        if (obj != _stb.noObj && obj->kind == OBJ_TYPE)
            return true;
    }

    return false;
}

bool CParser::isTypeQualifier(TokenKind kind)
{
    return kind == TK_CONST || kind == TK_VOLATILE;
}

bool CParser::isStorageClassSpecifier(TokenKind kind)
{
    return kind == TK_TYPEDEF || kind == TK_EXTERN || kind == TK_STATIC ||
           kind == TK_AUTO || kind == TK_REGISTER;
}

//
// Initialization
//

void CParser::initNames()
{
    _name.insert(std::make_pair(TK_UNKNOWN, "unknown"));
    _name.insert(std::make_pair(TK_IDENT, "identifier"));
    _name.insert(std::make_pair(TK_INT_LIT, "integer constant"));
    _name.insert(std::make_pair(TK_CHAR_LIT, "character constant"));
    _name.insert(std::make_pair(TK_FLOAT_LIT, "float constant"));
    _name.insert(std::make_pair(TK_STRING_LIT, "string constant"));
    _name.insert(std::make_pair(TK_SIZEOF, "sizeof"));
    _name.insert(std::make_pair(TK_PLUS, "+"));
    _name.insert(std::make_pair(TK_MINUS, "-"));
    _name.insert(std::make_pair(TK_TIMES, "*"));
    _name.insert(std::make_pair(TK_DIV, "/"));
    _name.insert(std::make_pair(TK_MOD, "%"));
    _name.insert(std::make_pair(TK_PERIODS, ".."));
    _name.insert(std::make_pair(TK_SEMICOLON, ";"));
    _name.insert(std::make_pair(TK_COLON, ":"));
    _name.insert(std::make_pair(TK_COMMA, ","));
    _name.insert(std::make_pair(TK_PERIOD, "."));
    _name.insert(std::make_pair(TK_LPAR, "("));
    _name.insert(std::make_pair(TK_RPAR, ")"));
    _name.insert(std::make_pair(TK_LBRACK, "["));
    _name.insert(std::make_pair(TK_RBRACK, "]"));
    _name.insert(std::make_pair(TK_LBRACE, "{"));
    _name.insert(std::make_pair(TK_RBRACE, "}"));
    _name.insert(std::make_pair(TK_TILDA, "~"));
    _name.insert(std::make_pair(TK_COND_OP, "?"));
    _name.insert(std::make_pair(TK_AND, "&"));
    _name.insert(std::make_pair(TK_LOGICAL_AND, "&&"));
    _name.insert(std::make_pair(TK_OR, "|"));
    _name.insert(std::make_pair(TK_LOGICAL_OR, "||"));
    _name.insert(std::make_pair(TK_EXCLUSIVE_OR, "^"));
    _name.insert(std::make_pair(TK_PTR_OP, "->"));
    _name.insert(std::make_pair(TK_INC_OP, "++"));
    _name.insert(std::make_pair(TK_DEC_OP, "--"));
    _name.insert(std::make_pair(TK_LSHIFT_OP, "<<"));
    _name.insert(std::make_pair(TK_RSHIFT_OP, ">>"));
    _name.insert(std::make_pair(TK_LSS, "<"));
    _name.insert(std::make_pair(TK_GTR, ">"));
    _name.insert(std::make_pair(TK_LEQ, "<="));
    _name.insert(std::make_pair(TK_GEQ, ">="));
    _name.insert(std::make_pair(TK_EQL, "=="));
    _name.insert(std::make_pair(TK_NEQ, "!="));
    _name.insert(std::make_pair(TK_NOT, "!"));
    _name.insert(std::make_pair(TK_ASSIGN, "="));
    _name.insert(std::make_pair(TK_MUL_ASSIGN, "*="));
    _name.insert(std::make_pair(TK_DIV_ASSIGN, "/="));
    _name.insert(std::make_pair(TK_MOD_ASSIGN, "%="));
    _name.insert(std::make_pair(TK_ADD_ASSIGN, "+="));
    _name.insert(std::make_pair(TK_SUB_ASSIGN, "-="));
    _name.insert(std::make_pair(TK_LSHIFT_ASSIGN, "<<"));
    _name.insert(std::make_pair(TK_RSHIFT_ASSIGN, ">>"));
    _name.insert(std::make_pair(TK_AND_ASSIGN, "&="));
    _name.insert(std::make_pair(TK_XOR_ASSIGN, "^="));
    _name.insert(std::make_pair(TK_OR_ASSIGN, "|="));
    _name.insert(std::make_pair(TK_TYPE_NAME, "type name"));
    _name.insert(std::make_pair(TK_ELLIPSIS, "..."));
    _name.insert(std::make_pair(TK_AUTO, "auto"));
    _name.insert(std::make_pair(TK_CHAR, "char"));
    _name.insert(std::make_pair(TK_CONST, "const"));
    _name.insert(std::make_pair(TK_DOUBLE, "double"));
    _name.insert(std::make_pair(TK_ELSE, "else"));
    _name.insert(std::make_pair(TK_ENUM, "enum"));
    _name.insert(std::make_pair(TK_EXTERN, "extern"));
    _name.insert(std::make_pair(TK_FLOAT, "float"));
    _name.insert(std::make_pair(TK_INT, "int"));
    _name.insert(std::make_pair(TK_LONG, "long"));
    _name.insert(std::make_pair(TK_REGISTER, "register"));
    _name.insert(std::make_pair(TK_SHORT, "short"));
    _name.insert(std::make_pair(TK_SIGNED, "signed"));
    _name.insert(std::make_pair(TK_STATIC, "static"));
    _name.insert(std::make_pair(TK_STRUCT, "struct"));
    _name.insert(std::make_pair(TK_TYPEDEF, "typedef"));
    _name.insert(std::make_pair(TK_UNION, "union"));
    _name.insert(std::make_pair(TK_UNSIGNED, "unsigned"));
    _name.insert(std::make_pair(TK_VOID, "void"));
    _name.insert(std::make_pair(TK_VOLATILE, "volatile"));
    _name.insert(std::make_pair(TK_ASM, "asm"));
    _name.insert(std::make_pair(TK_BREAK, "break"));
    _name.insert(std::make_pair(TK_CASE, "case"));
    _name.insert(std::make_pair(TK_CONTINUE, "continue"));
    _name.insert(std::make_pair(TK_DEFAULT, "default"));
    _name.insert(std::make_pair(TK_DO, "do"));
    _name.insert(std::make_pair(TK_FOR, "for"));
    _name.insert(std::make_pair(TK_GOTO, "goto"));
    _name.insert(std::make_pair(TK_IF, "if"));
    _name.insert(std::make_pair(TK_PUTC, "putc"));
    _name.insert(std::make_pair(TK_PUTI, "puti"));
    _name.insert(std::make_pair(TK_RETURN, "return"));
    _name.insert(std::make_pair(TK_SWITCH, "switch"));
    _name.insert(std::make_pair(TK_WHILE, "while"));
    _name.insert(std::make_pair(TK_EOF, "end of file"));
}

void CParser::parse(char *output, bool optimize)
{
    ASTNode *tree;
    int errors;

    _lex->nextCh();
    initTokenBuffer();
    _ast->setOptimize(optimize);

    tree = TranslationUnit();
    _ast->setRoot(tree);

    /*if (!parsingErrors)
    {
        if (asmOutput)
            _ast->getCodeGenerator()->setAsmOutput(true);
        errors = _ast->emitCode(output);
    }
    else
    {
        errors = parsingErrors;
    }

    if (!errors)
    {
        if (!_ast->getWarnings())
            printf("No errors; no warnings; lines: %d\n", _tok->line);
        else
            printf("No errors; warnings: %d; lines: %d\n", _ast->getWarnings(),
                   _tok->line);
        if (printAST)
            _ast->printTree();
        if (printIRET)
            _ast->printIRExprTree();
        if (!asmOutput && printCodeBuffer)
            _ast->getCodeGenerator()->decode();
    }
    else
    {
        printf("Errors: %d; warnings: %d\n", errors, _ast->getWarnings());
    }*/
}

//
//   T H E   P A R S E R   R U L E S
//

//
// Expressions
//

ASTNode *CParser::PrimaryExpression()
{
    ASTNode *res = NULL_AST_NODE;

    if (_sym == TK_IDENT)
    {
        getTok();
        res = new IdentASTNode(_tok->info.sval, _tok->line);
    }
    else if (_sym == TK_INT_LIT)
    {
        getTok();
        res = new IntegerConstASTNode(_tok->info.ival);
    }
    else if (_sym == TK_CHAR_LIT)
    {
        getTok();
        res = new CharConstASTNode(_tok->info.ival);
    }
    else if (_sym == TK_STRING_LIT)
    {
        getTok();
        res = new StringConstASTNode(_tok->info.sval);
    }
    else if (_sym == TK_FLOAT_LIT)
    {
        getTok();
        res = new RealConstASTNode(_tok->info.fval);
    }
    else if (_sym == TK_LPAR)
    {
        getTok();
        // Is this a function call? (* func)(x, y)
        if (_sym == TK_TIMES)
        {
            getTok();
            check(TK_IDENT);
            res = new IdentASTNode(_tok->info.sval, _tok->line);
        }
        else
        {
            res = Expression();
        }
        check(TK_RPAR);
    }
    return res;
}

//  a.b.c->d->e
//
//  =>
//       .
//      / \
//     a   .
//        / \
//       b  ->
//          / \
//         c  ->
//            / \
//           d   e
ASTNode *CParser::parsePostfixExpression(ASTNode *expr)
{
    if (_sym == TK_PERIOD)
    {
        ASTNode *tmp;

        getTok();
        check(TK_IDENT);
        tmp = new IdentASTNode(_tok->info.sval, _tok->line);
        return new StructRefASTNode(expr, parsePostfixExpression(tmp));
    }
    else if (_sym == TK_PTR_OP)
    {
        ASTNode *tmp;

        getTok();
        check(TK_IDENT);
        tmp = new IdentASTNode(_tok->info.sval, _tok->line);
        return new IndirectRefASTNode(NULL_AST_NODE, expr,
                                      parsePostfixExpression(tmp));
    }
    else
    {
        return expr;
    }
}

ASTNode *CParser::PostfixExpression(ASTNode *typeName)
{
    ASTNode *index = NULL_AST_NODE;
    ASTNode *expr = PrimaryExpression();

    for (;;)
    {
        if (_sym == TK_LBRACK)
        {
            getTok();
            index = Expression();
            expr = new ArrayRefASTNode(typeName, expr, index);
            check(TK_RBRACK);
        }
        else if (_sym == TK_LPAR)
        {
            ASTNode *args;
            getTok();

            args = NULL_AST_NODE;
            if (_sym != TK_RPAR)
                args = ArgumentExpressionList();
            check(TK_RPAR);
            expr = new CallExprASTNode(expr, args);
        }
        else if (_sym == TK_PERIOD)
        {
            ASTNode *tmp;

            getTok();
            check(TK_IDENT);
            tmp = new IdentASTNode(_tok->info.sval, _tok->line);
            expr = new StructRefASTNode(expr, parsePostfixExpression(tmp));
        }
        else if (_sym == TK_PTR_OP)
        { // '->'
            ASTNode *tmp;

            getTok();
            check(TK_IDENT);
            tmp = new IdentASTNode(_tok->info.sval, _tok->line);
            expr = new IndirectRefASTNode(NULL_AST_NODE, expr,
                                          parsePostfixExpression(tmp));
        }
        else if (_sym == TK_INC_OP)
        { // '++'
            getTok();
            expr = new PostincrementExprASTNode(expr);
        }
        else if (_sym == TK_DEC_OP)
        { // '--'
            getTok();
            expr = new PostdecrementExprASTNode(expr);
        }
        else
        {
            break;
        }
    }
    return expr;
}

ASTNode *CParser::ArgumentExpressionList()
{
    ListASTNode *args = new ListASTNode(AssignmentExpression());
    while (_sym == TK_COMMA)
    {
        getTok();
        args->add(AssignmentExpression());
    }
    return args;
}

ASTNode *CParser::UnaryExpression(ASTNode *typeName)
{
    ASTNode *res = NULL_AST_NODE;
    ASTNode *expr = NULL_AST_NODE;

    if ((_sym >= TK_IDENT && _sym <= TK_STRING_LIT) || _sym == TK_LPAR)
    {
        res = PostfixExpression(typeName);
    }
    else if (_sym == TK_INC_OP)
    {
        getTok();
        expr = UnaryExpression(typeName);
        res = new PreincrementExprASTNode(expr);
    }
    else if (_sym == TK_DEC_OP)
    {
        getTok();
        expr = UnaryExpression(typeName);
        res = new PredecrementExprASTNode(expr);
    }
    else if (_sym == TK_AND || _sym == TK_TIMES || _sym == TK_PLUS ||
             _sym == TK_MINUS || _sym == TK_TILDA || _sym == TK_NOT)
    {
        ASTNode *type = _ast->integerTypeASTNode;
        ASTNodeKind kind = UnaryOperator();

        if (kind == NK_INDIRECT_REF || kind == NK_ADDR_EXPR)
        {
            type = new PointerTypeASTNode(_ast->integerTypeASTNode);

            if (kind == NK_ADDR_EXPR)
                res = new AddrExprASTNode(type, CastExpression());
            else
                res = new IndirectRefASTNode(type, CastExpression(),
                                             NULL_AST_NODE);
        }
        else if (kind == NK_BIT_NOT_EXPR)
            res = new BitNotExprASTNode(type, CastExpression());
        else if (kind == NK_LOG_NOT_EXPR)
            res = new LogNotExprASTNode(type, CastExpression());
    }
    else if (_sym == TK_SIZEOF)
    {
        getTok();
        if (_sym == TK_LPAR)
        {
            getTok();
            //expr = TypeName();
            // FIXME: Put this into TypeName().
            expr = TypeSpecifier();
            if (_sym == TK_TIMES)
            {
                getTok();
                expr = new PointerTypeASTNode(expr);
            }
            check(TK_RPAR);
        }
        else
        {
            expr = UnaryExpression(NULL_AST_NODE);
        }
        res = new SizeOfExprASTNode(expr);
    }

    return res;
}

ASTNodeKind CParser::UnaryOperator()
{
    switch (_sym)
    {
        case TK_AND:
            getTok();
            return NK_ADDR_EXPR;
        case TK_TIMES:
            getTok();
            return NK_INDIRECT_REF;
        case TK_PLUS:
            getTok();
            return NK_UNKNOWN;
        case TK_MINUS:
            getTok();
            return NK_UNKNOWN;
        case TK_TILDA:
            getTok();
            return NK_BIT_NOT_EXPR;
        case TK_NOT:
            getTok();
            return NK_LOG_NOT_EXPR;
        default:
            return NK_UNKNOWN;
    }
}

ASTNode *CParser::CastExpression()
{
    ASTNode *typeName = NULL_AST_NODE;
    ASTNode *expr = NULL_AST_NODE;
    bool cast = false;
    bool isPtrType = false;

    if (_sym == TK_LPAR && isTypeSpecifier(getLATok(2), 2))
    {
        getTok();
        cast = true;
        typeName = TypeSpecifier();

        if (_sym == TK_TIMES)
        {
            getTok();
            isPtrType = true;
        }
        check(TK_RPAR);
    }
    expr = UnaryExpression(typeName);
    if (cast)
    {
        if (isPtrType)
            typeName = new PointerTypeASTNode(typeName);

        return new CastExprASTNode(typeName, expr);
    }
    else
    {
        return expr;
    }
}

ASTNode *CParser::MultiplicativeExpression()
{
    ASTNode *res = CastExpression();

    for (;;)
    {
        if (_sym == TK_TIMES)
        {
            getTok();
            res = AST_MULT(_ast->integerTypeASTNode, res, CastExpression());
        }
        else if (_sym == TK_DIV)
        {
            getTok();
            res = new TruncDivExprASTNode(_ast->integerTypeASTNode, res,
                                          CastExpression());
        }
        else if (_sym == TK_MOD)
        {
            getTok();
            res = new TruncModExprASTNode(_ast->integerTypeASTNode, res,
                                          CastExpression());
        }
        else
        {
            break;
        }
    }

    res->setLineNum(_tok->line);
    return res;
}

ASTNode *CParser::AdditiveExpression()
{
    ASTNode *res = MultiplicativeExpression();

    for (;;)
    {
        if (_sym == TK_PLUS)
        {
            getTok();
            res = AST_PLUS(_ast->integerTypeASTNode, res,
                           MultiplicativeExpression());
        }
        else if (_sym == TK_MINUS)
        {
            getTok();
            res = AST_MINUS(_ast->integerTypeASTNode, res,
                            MultiplicativeExpression());
        }
        else
        {
            break;
        }
    }
    res->setLineNum(_tok->line);

    return res;
}

ASTNode *CParser::ShiftExpression()
{
    ASTNode *res = AdditiveExpression();

    for (;;)
    {
        if (_sym == TK_LSHIFT_OP)
        {
            getTok();
            res = new LShiftExprASTNode(_ast->integerTypeASTNode, res,
                                        AdditiveExpression());
        }
        else if (_sym == TK_RSHIFT_OP)
        {
            getTok();
            res = new RShiftExprASTNode(_ast->integerTypeASTNode, res,
                                        AdditiveExpression());
        }
        else
        {
            break;
        }
    }
    res->setLineNum(_tok->line);

    return res;
}

ASTNode *CParser::RelationalExpression()
{
    ASTNode *res = ShiftExpression();

    for (;;)
    {
        if (_sym == TK_LSS)
        {
            getTok();
            res = new LtExprASTNode(_ast->integerTypeASTNode, res,
                                    ShiftExpression());
        }
        else if (_sym == TK_GTR)
        {
            getTok();
            res = new GtExprASTNode(_ast->integerTypeASTNode, res,
                                    ShiftExpression());
        }
        else if (_sym == TK_LEQ)
        {
            getTok();
            res = new LeExprASTNode(_ast->integerTypeASTNode, res,
                                    ShiftExpression());
        }
        else if (_sym == TK_GEQ)
        {
            getTok();
            res = new GeExprASTNode(_ast->integerTypeASTNode, res,
                                    ShiftExpression());
        }
        else
        {
            break;
        }
    }
    res->setLineNum(_tok->line);

    return res;
}

ASTNode *CParser::EqualityExpression()
{
    ASTNode *res = RelationalExpression();

    for (;;)
    {
        if (_sym == TK_EQL)
        {
            getTok();
            res = new EqExprASTNode(_ast->integerTypeASTNode, res,
                                    RelationalExpression());
        }
        else if (_sym == TK_NEQ)
        {
            getTok();
            res = new NeExprASTNode(_ast->integerTypeASTNode, res,
                                    RelationalExpression());
        }
        else
        {
            break;
        }
    }
    res->setLineNum(_tok->line);

    return res;
}

ASTNode *CParser::AndExpression()
{
    ASTNode *res = EqualityExpression();

    while (_sym == TK_AND)
    {
        getTok();
        res = new BitAndExprASTNode(_ast->integerTypeASTNode, res,
                                    EqualityExpression());
    }
    res->setLineNum(_tok->line);

    return res;
}

ASTNode *CParser::ExclusiveOrExpression()
{
    ASTNode *res = AndExpression();

    while (_sym == TK_EXCLUSIVE_OR)
    {
        getTok();
        res = new BitXorExprASTNode(_ast->integerTypeASTNode, res,
                                    AndExpression());
    }
    res->setLineNum(_tok->line);

    return res;
}

ASTNode *CParser::InclusiveOrExpression()
{
    ASTNode *res = ExclusiveOrExpression();

    while (_sym == TK_OR)
    {
        getTok();
        res = new BitIorExprASTNode(_ast->integerTypeASTNode, res,
                                    ExclusiveOrExpression());
    }
    res->setLineNum(_tok->line);

    return res;
}

ASTNode *CParser::LogicalAndExpression()
{
    ASTNode *res = InclusiveOrExpression();

    while (_sym == TK_LOGICAL_AND)
    {
        getTok();
        res = new LogAndExprASTNode(_ast->integerTypeASTNode, res,
                                    InclusiveOrExpression());
    }
    res->setLineNum(_tok->line);

    return res;
}

ASTNode *CParser::LogicalOrExpression()
{
    ASTNode *res = LogicalAndExpression();

    while (_sym == TK_LOGICAL_OR)
    {
        getTok();
        res = new LogOrExprASTNode(_ast->integerTypeASTNode, res,
                                   LogicalAndExpression());
    }
    res->setLineNum(_tok->line);

    return res;
}

ASTNode *CParser::ConditionalExpression()
{
    ASTNode *expr = NULL_AST_NODE;
    ASTNode *res = LogicalOrExpression();

    if (_sym == TK_COND_OP)
    {
        getTok();
        expr = Expression();
        check(TK_COLON);
        res = new CondExprASTNode(res, expr, ConditionalExpression());
    }
    res->setLineNum(_tok->line);

    return res;
}

ASTNode *CParser::AssignmentExpression()
{
    ASTNode *res = ConditionalExpression();

    if (_sym >= TK_ASSIGN && _sym <= TK_OR_ASSIGN)
    {
        unsigned op = _sym;

        getTok();
        if (op >= TK_MUL_ASSIGN && op <= TK_OR_ASSIGN)
        {
            ASTNode *lhs = res;
            ASTNode *rhs = AssignmentExpression();
            ASTNode *expr = NULL_AST_NODE;

            switch (op)
            {
                case TK_MUL_ASSIGN:
                    expr = AST_MULT(_ast->integerTypeASTNode, lhs, rhs);
                    break;
                case TK_DIV_ASSIGN:
                    expr = new TruncDivExprASTNode(_ast->integerTypeASTNode, lhs,
                                                   rhs);
                    break;
                case TK_MOD_ASSIGN:
                    expr = new TruncModExprASTNode(_ast->integerTypeASTNode, lhs,
                                                   rhs);
                    break;
                case TK_ADD_ASSIGN:
                    expr = AST_PLUS(_ast->integerTypeASTNode, lhs, rhs);
                    break;
                case TK_SUB_ASSIGN:
                    expr = AST_MINUS(_ast->integerTypeASTNode, lhs, rhs);
                    break;
                case TK_LSHIFT_ASSIGN:
                    expr = new LShiftExprASTNode(_ast->integerTypeASTNode, lhs,
                                                 rhs);
                    break;
                case TK_RSHIFT_ASSIGN:
                    expr = new RShiftExprASTNode(_ast->integerTypeASTNode, lhs,
                                                 rhs);
                    break;
                case TK_AND_ASSIGN:
                    expr = new BitAndExprASTNode(_ast->integerTypeASTNode, lhs,
                                                 rhs);
                    break;
                case TK_XOR_ASSIGN:
                    expr = new BitXorExprASTNode(_ast->integerTypeASTNode, lhs,
                                                 rhs);
                    break;
                case TK_OR_ASSIGN:
                    expr = new BitIorExprASTNode(_ast->integerTypeASTNode, lhs,
                                                 rhs);
                    break;
            }
            res = new AssignExprASTNode(_ast->integerTypeASTNode, res, expr);
        }
        else
        {
            res = new AssignExprASTNode(_ast->integerTypeASTNode, res,
                                        AssignmentExpression());
        }
    }
    res->setLineNum(_tok->line);

    return res;
}

ASTNode *CParser::Expression()
{
    ASTNode *expr = AssignmentExpression();

    while (_sym == TK_COMMA)
    {
        getTok();
        AssignmentExpression();
    }

    return expr;
}

ASTNode *CParser::ConstantExpression() { return ConditionalExpression(); }

//
// Declarations
//

ASTNode *CParser::Declaration(ASTNode *declSpec)
{
    ASTNode *initlist = NULL_AST_NODE;

    if (_sym != TK_SEMICOLON)
        initlist = InitDeclaratorList(declSpec);

    check(TK_SEMICOLON);

    if (initlist != NULL_AST_NODE)
    {
        //initlist->transform(NK_VAR_DECL, declspec);
        return initlist;
    }
    else
    {
        // Here is a forward struct declaration so far.
        return declSpec;
    }
}

ASTNode *CParser::DeclarationSpecifiers()
{
    ASTNode *declSpec = NULL_AST_NODE;
    ASTNodeKind storageClassSpec = NK_UNKNOWN;
    unsigned flags = 0x0;

    if (isStorageClassSpecifier(_sym))
        StorageClassSpecifier(flags);

    if (isTypeQualifier(_sym))
        TypeQualifier(flags);

    if (isTypeSpecifier(_sym, 1))
    {
        declSpec = TypeSpecifier();
        declSpec->setFlags(flags);
    }

    return declSpec;
}

ListASTNode *CParser::InitDeclaratorList(ASTNode *typeSpec)
{
    ListASTNode *initlist = new ListASTNode(InitDeclarator(typeSpec));

    while (_sym == TK_COMMA)
    {
        getTok();
        initlist->add(InitDeclarator(typeSpec));
    }

    return initlist;
}

ASTNode *CParser::InitDeclarator(ASTNode *typeSpec)
{
    ASTNode *init = NULL_AST_NODE;
    ASTNode *declr = Declarator(DK_VARIABLE, typeSpec);

    if (_sym == TK_ASSIGN)
    {
        getTok();
        init = Initializer();
        if (declr->getKind() == NK_VAR_DECL)
        {
            VarDeclASTNode *varDecl = static_cast<VarDeclASTNode *>(declr);
            varDecl->setInit(init);
        }
    }

    return declr;
}

void CParser::StorageClassSpecifier(unsigned &flags)
{
    if (_sym == TK_TYPEDEF)
    {
        getTok();
        flags |= SCS_TYPEDEF;
    }
    else if (_sym == TK_EXTERN)
    {
        getTok();
        flags |= SCS_EXTERN;
    }
    else if (_sym == TK_STATIC)
    {
        getTok();
        flags |= SCS_STATIC;
    }
    else if (_sym == TK_AUTO)
    {
        getTok();
        flags |= SCS_AUTO;
    }
    else if (_sym == TK_REGISTER)
    {
        getTok();
        flags |= SCS_REGISTER;
    }
}

static ASTNode *
stbTypeToASTNodeType(AbstractSyntaxTree *ast, Type_t type, const char *objName)
{
    switch (type->kind)
    {
        case T_NONE:
            return NULL_AST_NODE;
        case T_CHAR:
            return ast->charTypeASTNode;
        case T_SHORT:
            return ast->shortTypeASTNode;
        case T_INT:
            return ast->integerTypeASTNode;
        case T_UNSIGNED:
            return ast->unsignedTypeASTNode;
        case T_LONG:
            // return _ast->longTypeASTNode;
            return NULL_AST_NODE;
        case T_FLOAT:
            return ast->floatTypeASTNode;
        case T_DOUBLE:
            // return IR_f64;
            return NULL_AST_NODE;
        case T_VOID:
            return ast->voidTypeASTNode;
        case T_ARRAY:
            //  return IR_i32;
            return NULL_AST_NODE;
        case T_STRUCT:
            // return _ast->createStructTypeASTNode(new IdentASTNode(objName),
            //                                    NULL_AST_NODE);
            return new StructTypeASTNode(new IdentASTNode(objName),
                                         NULL_AST_NODE);
        case T_UNION:
            return new UnionTypeASTNode(new IdentASTNode(objName),
                                        NULL_AST_NODE);
        case T_BOOL:
            return NULL_AST_NODE;
        case T_REAL:
            return NULL_AST_NODE;
        case T_POINTER:
            return new PointerTypeASTNode(
                stbTypeToASTNodeType(ast, type->baseType, objName));
        case T_ENUM:
            return NULL_AST_NODE;
        case T_FUNCTION:
            return NULL_AST_NODE;
    }
    return NULL_AST_NODE;
}

ASTNode *CParser::TypeSpecifier()
{
    if (_sym == TK_VOID)
    {
        getTok();
        return _ast->voidTypeASTNode;
    }
    else if (_sym == TK_CHAR)
    {
        getTok();
        return _ast->charTypeASTNode;
    }
    else if (_sym == TK_SHORT)
    {
        getTok();
        return _ast->shortTypeASTNode;
    }
    else if (_sym == TK_INT)
    {
        getTok();
        return _ast->integerTypeASTNode;
    }
    else if (_sym == TK_LONG)
    {
        getTok();
    }
    else if (_sym == TK_FLOAT)
    {
        getTok();
        return _ast->floatTypeASTNode;
    }
    else if (_sym == TK_DOUBLE)
    {
        getTok();
    }
    else if (_sym == TK_SIGNED)
    {
        getTok();
    }
    else if (_sym == TK_UNSIGNED)
    {
        getTok();
        return _ast->unsignedTypeASTNode;
    }
    else if (_sym == TK_STRUCT || _sym == TK_UNION)
    {
        return StructOrUnionSpecifier();
    }
    else if (_sym == TK_ENUM)
    {
        return EnumSpecifier();
    }
    else if (_sym == TK_IDENT)
    { // TYPE_NAME
        char msg[MAXSTR];
        Object_t obj;

        getTok();

        obj = _stb.find(_tok->info.sval);
        if (obj != _stb.noObj)
        {
            return stbTypeToASTNodeType(_ast, obj->type, obj->name);
        }
        snprintf(msg, MAXSTR, "unknown type '%s'", obj->name);
        parsingError(msg);
    }

    return NULL_AST_NODE;
}

// TODO: Make distinct productions for a struct type declaration and for
//       declaration of variable of a struct type.
ASTNode *CParser::StructOrUnionSpecifier()
{
    ASTNode *typeName = NULL_AST_NODE;
    ASTNode *typeBody = NULL_AST_NODE;
    Object_t obj = nullptr;
    ASTNodeKind typeKind;

    if (_sym == TK_STRUCT)
    {
        getTok();
        typeKind = NK_STRUCT_TYPE;
    }
    else
    { // _sym == UNION
        getTok();
        typeKind = NK_UNION_TYPE;
    }

    if (_sym == TK_IDENT)
    {
        Type_t recordType;

        getTok();
        typeName = new IdentASTNode(_tok->info.sval, _tok->line);
        if (typeKind == NK_STRUCT_TYPE)
            recordType = _stb.allocType(T_STRUCT);
        else
            recordType = _stb.allocType(T_UNION);
        obj = _stb.find(_tok->info.sval);
        /*if (obj != _stb.noObj && obj->type->fields != NULL)
        {
            _ast->error("struct type '%s' has already been declared",
                       _tok->info.sval,
                       _tok->line);
        }
        else */
        if (obj == _stb.noObj)
            obj = _stb.insert(_tok->info.sval, OBJ_TYPE, recordType);
    }
    if (_sym == TK_LBRACE)
    {
        getTok();
        _stb.openScope();
        typeBody = StructDeclarationList();
        // FIXME: What if we have anonymous struct type?
        if (obj != nullptr)
        {
            if (typeKind == NK_UNION_TYPE)
            {
                Object_t tmp = _stb.getTopScope()->locals;
                unsigned maxFieldTypeSize = 0;

                while (tmp != nullptr)
                {
                    if (maxFieldTypeSize < tmp->type->size)
                        maxFieldTypeSize = tmp->type->size;
                    tmp->adr = 0;
                    tmp = tmp->next;
                }
                _stb.getTopScope()->size = maxFieldTypeSize;
            }
            obj->type->fields = _stb.getTopScope()->locals;
            obj->type->size = _stb.getTopScope()->size;
        }
        _stb.closeScope();
        check(TK_RBRACE);
    }
    if (typeKind == NK_STRUCT_TYPE)
        // return _ast->createStructTypeASTNode(typeName, typeBody);
        return new StructTypeASTNode(typeName, typeBody);
    else
        return new UnionTypeASTNode(typeName, typeBody);
}

ListASTNode *CParser::StructDeclarationList()
{
    ListASTNode *declist = new ListASTNode(StructDeclaration());

    while (_sym != TK_RBRACE)
        declist->add(StructDeclaration());

    return declist;
}

ASTNode *CParser::StructDeclaration()
{
    ASTNode *typeSpec = TypeSpecifier();
    ListASTNode *declaratorList = StructDeclaratorList(typeSpec);

    declaratorList->declare(_ast);
    check(TK_SEMICOLON);

    return declaratorList;
}

ListASTNode *CParser::StructDeclaratorList(ASTNode *typeSpec)
{
    ListASTNode *declist = new ListASTNode(StructDeclarator(typeSpec));

    while (_sym == TK_COMMA)
    {
        getTok();
        declist->add(StructDeclarator(typeSpec));
    }

    return declist;
}

ASTNode *CParser::StructDeclarator(ASTNode *typeSpec)
{
    return Declarator(DK_FIELD, typeSpec);
}

ASTNode *CParser::EnumSpecifier()
{
    ASTNode *name = NULL_AST_NODE;
    ASTNode *body = NULL_AST_NODE;

    getTok(); // Eat 'enum'.

    if (_sym == TK_IDENT)
    {
        Type_t enumType;

        getTok();
        name = new IdentASTNode(_tok->info.sval);
        enumType = _stb.allocType(T_ENUM);
        _stb.insert(_tok->info.sval, OBJ_TYPE, enumType);
    }

    if (_sym == TK_LBRACE)
    {
        getTok();
        body = EnumeratorList();
        check(TK_RBRACE);
    }

    return new EnumeralTypeASTNode(name, body);
}

ListASTNode *CParser::EnumeratorList()
{
    ListASTNode *enums;
    Object_t obj;
    unsigned i = 0;

    check(TK_IDENT);
    enums = new ListASTNode(new IdentASTNode(_tok->info.sval));

    obj = _stb.insert(_tok->info.sval, OBJ_CON, _stb.intType);
    obj->ival = i;

    if (_sym == TK_ASSIGN)
    {
        getTok();
        ConstantExpression();
    }

    while (_sym == TK_COMMA)
    {
        getTok();
        check(TK_IDENT);
        enums->add(new IdentASTNode(_tok->info.sval));

        i++;
        obj = _stb.insert(_tok->info.sval, OBJ_CON, _stb.intType);
        obj->ival = i;

        if (_sym == TK_ASSIGN)
        {
            getTok();
            ConstantExpression();
        }
    }

    return enums;
}

void CParser::TypeQualifier(unsigned &flags)
{
    if (_sym == TK_CONST)
    {
        getTok();
        flags |= TQ_CONST;
    }
    else // TK_VOLATILE
    {
        getTok();
        flags |= TQ_VOLATILE;
    }
}

ASTNode *CParser::Declarator(DeclaratorKind declKind, ASTNode *typeSpec)
{
    while (_sym == TK_TIMES)
    {
        unsigned flags;

        getTok();

        // Flags needs to be preserved
        flags = typeSpec->getFlags();
        typeSpec = new PointerTypeASTNode(typeSpec);
        typeSpec->setFlags(flags);
    }

    return DirectDeclarator(declKind, typeSpec);
}

ASTNode *CParser::DirectDeclarator(DeclaratorKind declKind, ASTNode *typeSpec)
{
    ASTNode *declr = NULL_AST_NODE;
    ASTNode *expr = NULL_AST_NODE;
    FunctionTypeASTNode *funcType;
    bool isFuncPtr = false;

    if (_sym == TK_IDENT)
    {
        getTok();
        declr = new IdentASTNode(_tok->info.sval, _tok->line);
    }
    else
    {
        check(TK_LPAR);
        if (_sym == TK_TIMES)
        {
            getTok();
            // This is a function pointer.
            isFuncPtr = true;
            // FIXME: Make this direct type without T_POINTER.
            funcType = new FunctionTypeASTNode(typeSpec, NULL_AST_NODE);
            typeSpec = new PointerTypeASTNode(funcType);
        }
        declr = Declarator(DK_IDENT, NULL_AST_NODE);
        check(TK_RPAR);
    }
    for (;;)
    {
        if (_sym == TK_LBRACK)
        {
            getTok();

            if (_sym != TK_RBRACK)
            {
                expr = Expression();
                typeSpec = new ArrayTypeASTNode(typeSpec, expr);
            }
            else
            {
                // Declarations like int a[] are treated as a pointers.
                typeSpec = new PointerTypeASTNode(typeSpec);
            }
            check(TK_RBRACK);
        } /* else if (_sym == TK_LPAR)
         {
             getTok();
             if (_sym != TK_RPAR)
             {
                 //
             }
             check(TK_RPAR);
         }*/
        else
        {
            break;
        }
    }
    if (_sym == TK_LPAR && isFuncPtr)
    {
        getTok();
        if (_sym != TK_RPAR)
        {
            ASTNode *funcPrms = ParameterTypeList();
            funcType->setPrms(funcPrms);
        }
        check(TK_RPAR);
    }
    if ((typeSpec != NULL_AST_NODE) &&
        ((typeSpec->getFlags() & SCS_TYPEDEF) != 0))
    {
        declr = new TypeDeclASTNode(declr, typeSpec);
    }
    else
    {
        switch (declKind)
        {
            case DK_VARIABLE:
                declr = new VarDeclASTNode(typeSpec, declr);
                break;
            case DK_PARAMETER:
                declr = new ParmDeclASTNode(typeSpec, declr);
                break;
            case DK_FIELD:
                declr = new FieldDeclASTNode(typeSpec, declr);
                break;
            default:
                break;
        }
    }

    return declr;
}

ASTNode *CParser::ParameterTypeList() { return ParameterList(); }

ASTNode *CParser::ParameterList()
{
    ListASTNode *prms = new ListASTNode(ParameterDeclaration());

    while (_sym == TK_COMMA)
    {
        getTok();
        prms->add(ParameterDeclaration());
    }
    return prms;
}

ASTNode *CParser::ParameterDeclaration()
{
    ASTNode *typeSpec = TypeSpecifier();
    ASTNode *declr = Declarator(DK_PARAMETER, typeSpec);

    return declr;
}

ASTNode *CParser::IdentifierList()
{
    ListASTNode *res;

    check(TK_IDENT);
    res = new ListASTNode(new IdentASTNode(_tok->info.sval, _tok->line));

    while (_sym == TK_COMMA)
    {
        getTok();
        check(TK_IDENT);
        res->add(new IdentASTNode(_tok->info.sval, _tok->line));
    }

    return res;
}

ASTNode *CParser::TypeName()
{
    check(TK_IDENT);
    return new IdentASTNode(_tok->info.sval, _tok->line);
}

ASTNode *CParser::Initializer()
{
    ASTNode *res = NULL_AST_NODE;

    if (_sym == TK_LBRACE)
    {
        getTok();

        res = InitializerList();

        check(TK_RBRACE);
    }
    else
    {
        res = AssignmentExpression();
    }

    return res;
}

ASTNode *CParser::InitializerList()
{
    ListASTNode *res;
    ASTNode *init;

    init = Initializer();
    res = new ListASTNode(init);

    while (_sym == TK_COMMA)
    {
        getTok();

        init = Initializer();
        res->add(init);
    }

    return res;
}

//
// Statements
//

ASTNode *CParser::Statement()
{
    ASTNode *stmt = NULL_AST_NODE;
    ASTNode *expr = NULL_AST_NODE;

    if (_sym == TK_IDENT)
    {
        if (getLATok(2) == TK_COLON)
            stmt = LabeledStatement();
        else
            stmt = ExpressionStatement();
    }
    else if (_sym == TK_TIMES || (_sym == TK_LPAR && getLATok(2) == TK_TIMES))
    {
        // This is a function call of type: (*func)(x, y)
        stmt = ExpressionStatement();
    }
    else if (_sym == TK_CASE || _sym == TK_DEFAULT)
    {
        stmt = LabeledStatement();
    }
    else if (_sym == TK_LBRACE)
    {
        stmt = CompoundStatement();
    }
    else if (_sym == TK_IF || _sym == TK_SWITCH)
    {
        stmt = SelectionStatement();
    }
    else if (_sym == TK_WHILE || _sym == TK_DO || _sym == TK_FOR)
    {
        stmt = IterationStatement();
    }
    else if (_sym == TK_GOTO || _sym == TK_CONTINUE || _sym == TK_BREAK ||
             _sym == TK_RETURN)
    {
        stmt = JumpStatement();
    }
    else if (_sym == TK_PUTC)
    {
        getTok();
        check(TK_LPAR);
        expr = Expression();
        stmt = new PutCharExprASTNode(expr);
        check(TK_RPAR);
        check(TK_SEMICOLON);
    }
    else if (_sym == TK_PUTI)
    {
        getTok();
        check(TK_LPAR);
        expr = Expression();
        stmt = new PutIntExprASTNode(expr);
        check(TK_RPAR);
        check(TK_SEMICOLON);
    }
    else if (_sym == TK_ASM)
    {
        getTok();
        check(TK_LPAR);
        check(TK_STRING_LIT);
        stmt = new AsmStmtASTNode(_tok->info.sval);
        if (_sym == TK_COLON)
        {
            // Output operands
            getTok();
            if (_sym != TK_COLON)
            {
                check(TK_STRING_LIT);
                check(TK_LPAR);
                Expression();
                check(TK_RPAR);
                while (_sym == TK_COMMA)
                {
                    getTok();
                    check(TK_STRING_LIT);
                    check(TK_LPAR);
                    Expression();
                    check(TK_RPAR);
                }
            }
        }
        if (_sym == TK_COLON)
        {
            // Input operands
            getTok();
            if (_sym != TK_COLON)
            {
                check(TK_STRING_LIT);
                check(TK_LPAR);
                Expression();
                check(TK_RPAR);
                while (_sym == TK_COMMA)
                {
                    getTok();
                    check(TK_STRING_LIT);
                    check(TK_LPAR);
                    Expression();
                    check(TK_RPAR);
                }
            }
        }
        if (_sym == TK_COLON)
        {
            // Clobbered inputs
            getTok();
            if (_sym != TK_RPAR)
            {
                check(TK_STRING_LIT);
                while (_sym == TK_COMMA)
                {
                    getTok();
                    check(TK_STRING_LIT);
                }
            }
        }
        check(TK_RPAR);
        check(TK_SEMICOLON);
    }

    return stmt;
}

ASTNode *CParser::LabeledStatement()
{
    ASTNode *labstmt = NULL_AST_NODE;
    ASTNode *label = NULL_AST_NODE;
    ASTNode *stmt = NULL_AST_NODE;

    if (_sym == TK_IDENT)
    {
        getTok();
        label = new IdentASTNode(_tok->info.sval, _tok->line);
        check(TK_COLON);
        stmt = Statement();
        labstmt = new LabelStmtASTNode(label, stmt);
    }
    else if (_sym == TK_CASE)
    {
        ASTNode *expr;

        getTok();
        expr = ConstantExpression();
        check(TK_COLON);
        stmt = Statement();
        labstmt = new CaseLabelASTNode(label, stmt);
    }
    else if (_sym == TK_DEFAULT)
    {
        getTok();
        check(TK_COLON);
        Statement();
    }

    return labstmt;
}

ASTNode *CParser::CompoundStatement()
{
    ASTNode *compstmt = NULL_AST_NODE;

    check(TK_LBRACE);
    if (_sym != TK_RBRACE)
        compstmt = StmtOrDeclList();
    check(TK_RBRACE);

    return compstmt;
}

ASTNode *CParser::FunctionBody()
{
    if (_sym != TK_LBRACE)
    { // Forward declaration
        return NULL_AST_NODE;
    }

    ASTNode *decl = NULL_AST_NODE;
    ASTNode *stmt = NULL_AST_NODE;
    ASTNode *declList = NULL_AST_NODE;
    ASTNode *stmtList = NULL_AST_NODE;
    ASTNode *compound = NULL_AST_NODE;

    getTok(); // eat '{'
    compound = new CompoundStmtASTNode(NULL_AST_NODE, NULL_AST_NODE);
    if (_sym != TK_RBRACE)
    {
        // The scope has been opened in FunctionDeclASTNode.

        // Declarations
        while (isStorageClassSpecifier(_sym) || isTypeQualifier(_sym) ||
               isTypeSpecifier(_sym, 1))
        {
            ASTNode *declSpec = DeclarationSpecifiers();

            decl = Declaration(declSpec);
            _ast->declare(decl);
            if (declList == NULL_AST_NODE)
                declList = new ListASTNode(decl);
            else
                static_cast<ListASTNode *>(declList)->add(decl);
        }

        // Statements
        for (;;)
        {
            if (_sym == TK_IDENT || _sym == TK_TIMES ||
                (_sym == TK_LPAR && getLATok(2) == TK_TIMES))
            {
                stmt = Statement();
                if (stmtList == NULL_AST_NODE)
                    stmtList = new ListASTNode(stmt);
                else
                    static_cast<ListASTNode *>(stmtList)->add(stmt);
            }
            else if (_sym >= TK_ASM && _sym <= TK_WHILE)
            {
                stmt = Statement();
                if (stmtList == NULL_AST_NODE)
                    stmtList = new ListASTNode(stmt);
                else
                    static_cast<ListASTNode *>(stmtList)->add(stmt);
            }
            else
            {
                break;
            }
        }
        static_cast<CompoundStmtASTNode *>(compound)->setDecls(declList);
        static_cast<CompoundStmtASTNode *>(compound)->setStmts(stmtList);
    }
    else
    {
        static_cast<CompoundStmtASTNode *>(compound)->setStmts(
            new ListASTNode(new NopExprASTNode()));
    }
    check(TK_RBRACE);

    return compound;
}

ASTNode *CParser::DeclarationList()
{
    // Declaration()
    return NULL_AST_NODE;
}

ASTNode *CParser::StmtOrDeclList()
{
    ASTNode *decl = NULL_AST_NODE;
    ASTNode *stmt = NULL_AST_NODE;
    ASTNode *stmts = NULL_AST_NODE;
    ASTNode *decls = NULL_AST_NODE;

    // Declarations
    while (isStorageClassSpecifier(_sym) || isTypeQualifier(_sym) ||
           isTypeSpecifier(_sym, 1))
    {
        ASTNode *declSpec = DeclarationSpecifiers();

        decl = Declaration(declSpec);
        _ast->declare(decl);
        if (decls == NULL_AST_NODE)
            decls = new ListASTNode(decl);
        else
            static_cast<ListASTNode *>(decls)->add(decl);
    }

    // Statements
    for (;;)
    {
        if (_sym == TK_IDENT)
        {
            stmt = Statement();
            if (stmts == NULL_AST_NODE)
                stmts = new ListASTNode(stmt);
            else
                static_cast<ListASTNode *>(stmts)->add(stmt);
        }
        else if (_sym >= TK_ASM && _sym <= TK_WHILE)
        {
            stmt = Statement();
            if (stmts == NULL_AST_NODE)
                stmts = new ListASTNode(stmt);
            else
                static_cast<ListASTNode *>(stmts)->add(stmt);
        }
        else
        {
            break;
        }
    }

    return new CompoundStmtASTNode(decls, stmts);
}

ASTNode *CParser::SelectionStatement()
{
    ASTNode *selstmt = NULL_AST_NODE;

    if (_sym == TK_IF)
    {
        ASTNode *expr;
        ASTNode *then_clause;
        ASTNode *else_clause = NULL_AST_NODE;

        getTok();
        check(TK_LPAR);
        expr = Expression();
        check(TK_RPAR);
        then_clause = Statement();

        if (_sym == TK_ELSE)
        {
            getTok();
            else_clause = Statement();
        }

        selstmt = new IfStmtASTNode(expr, then_clause, else_clause);
    }
    else if (_sym == TK_SWITCH)
    {
        ASTNode *expr;
        ASTNode *stmt;

        getTok();
        check(TK_LPAR);
        expr = Expression();
        check(TK_RPAR);
        stmt = Statement();

        selstmt = new SwitchStmtASTNode(expr, stmt);
    }

    return selstmt;
}

ASTNode *CParser::IterationStatement()
{
    ASTNode *iterstmt = NULL_AST_NODE;
    ASTNode *expr = NULL_AST_NODE;
    ASTNode *stmt = NULL_AST_NODE;

    if (_sym == TK_WHILE)
    {
        getTok();
        check(TK_LPAR);

        // Get condition
        expr = Expression();
        check(TK_RPAR);

        // Get while body
        stmt = Statement();

        // Create while statement
        iterstmt = new WhileStmtASTNode(expr, stmt);
    }
    else if (_sym == TK_DO)
    {
        getTok();

        // Get do-while body
        stmt = Statement();
        check(TK_WHILE);
        check(TK_LPAR);

        // Get condition
        expr = Expression();
        check(TK_RPAR);
        check(TK_SEMICOLON);

        // Create while statement
        iterstmt = new DoStmtASTNode(expr, stmt);
    }
    else if (_sym == TK_FOR)
    {
        ASTNode *step = NULL_AST_NODE;
        ASTNode *init = NULL_AST_NODE;

        getTok();
        check(TK_LPAR);

        if (_sym != TK_SEMICOLON)
            init = Expression();

        check(TK_SEMICOLON);

        if (_sym != TK_SEMICOLON)
            expr = Expression();

        check(TK_SEMICOLON);

        if (_sym != TK_RPAR)
            step = Expression();

        check(TK_RPAR);

        stmt = Statement();

        // Create for statement
        iterstmt = new ForStmtASTNode(init, expr, step, stmt);
    }

    return iterstmt;
}

ASTNode *CParser::JumpStatement()
{
    ASTNode *jumpstmt = NULL_AST_NODE;
    ASTNode *expr = NULL_AST_NODE;

    if (_sym == TK_GOTO)
    {
        getTok();
        check(TK_IDENT);
        jumpstmt =
            new GotoStmtASTNode(new IdentASTNode(_tok->info.sval, _tok->line));
        check(TK_SEMICOLON);
    }
    else if (_sym == TK_CONTINUE)
    {
        getTok();
        jumpstmt = new ContinueStmtASTNode();
        check(TK_SEMICOLON);
    }
    else if (_sym == TK_BREAK)
    {
        getTok();
        jumpstmt = new BreakStmtASTNode();
        check(TK_SEMICOLON);
    }
    else if (_sym == TK_RETURN)
    {
        getTok();
        if (_sym != TK_SEMICOLON)
            expr = Expression();
        check(TK_SEMICOLON);
        jumpstmt = new ReturnStmtASTNode(_ast->integerTypeASTNode, expr);
        jumpstmt->setLineNum(_tok->line);
    }

    return jumpstmt;
}

ASTNode *CParser::ExpressionStatement()
{
    ASTNode *expr = NULL_AST_NODE;

    if (_sym != TK_SEMICOLON)
        expr = Expression();
    check(TK_SEMICOLON);

    return expr;
}

//
// Translation unit
//

ASTNode *CParser::TranslationUnit()
{
    ASTNode *extdecl = NULL_AST_NODE;
    ASTNode *tunit = NULL_AST_NODE;

    _stb.openScope();

    for (;;)
    {
        if (isStorageClassSpecifier(_sym) || isTypeQualifier(_sym) ||
            isTypeSpecifier(_sym, 1))
        {
            extdecl = ExternalDeclaration();
            if (tunit == NULL_AST_NODE)
                tunit = new ListASTNode(extdecl);
            else
                static_cast<ListASTNode *>(tunit)->add(extdecl);
        }
        else
        {
            break;
        }
    }

    static_cast<ListASTNode *>(tunit)->setScope(_stb.getTopScope());
    _stb.closeScope();

    return tunit;
}

ASTNode *CParser::ExternalDeclaration()
{
    ASTNode *declSpec = DeclarationSpecifiers();

    if (getLATok(2) == TK_LPAR || getLATok(3) == TK_LPAR)
    {
        return FunctionDefinition(declSpec);
    }
    else
    {
        ASTNode *decl = Declaration(declSpec);

        // Traverse declarations i.e. insert into symbol table.
        _ast->declare(decl);

        return decl;
    }
}

ASTNode *CParser::FunctionDefinition(ASTNode *funcType)
{
    ASTNode *funcName = NULL_AST_NODE;
    ASTNode *funcPrms = NULL_AST_NODE;
    bool isPtrType = false;

    if (_sym == TK_TIMES)
    {
        getTok();
        funcType = new PointerTypeASTNode(funcType);
    }

    // First parse both the function name and the parameters (if any) and after
    // that declare them. Function cannot be declared before parameters are
    // parsed because we have to know the number of parameters, to match
    // regular declarations against coresponding forward ones.
    //
    // FIXME: We need to check not only number of parameters but also their
    // types.

    // Function name
    funcName = Declarator(DK_IDENT, NULL_AST_NODE);

    check(TK_LPAR);

    FunctionDeclASTNode *funcDecl = new FunctionDeclASTNode(
        funcType, funcName, NULL_AST_NODE /* parameters */,
        NULL_AST_NODE /* body */);

    if (_sym != TK_RPAR)
    {
        funcPrms = ParameterTypeList();
        funcDecl->setPrms(funcPrms);
    }
    check(TK_RPAR);

    _ast->declare(funcDecl);

    _stb.openScope();

    // Parameters are going inside of the function body.
    if (funcPrms != NULL_AST_NODE)
        _ast->declare(funcPrms);

    if (_sym != TK_SEMICOLON)
        funcDecl->setBody(FunctionBody());
    else
        getTok();

    funcDecl->setScope(_stb.getTopScope());
    _stb.closeScope();
    _ast->setCurrentFuncDecl(nullptr);

    return funcDecl;
}
