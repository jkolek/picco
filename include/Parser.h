// PICCO parser - header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef PARSER_H
#define PARSER_H

#include <map>

#include "AbstractSyntaxTree.h"
#include "Lexer.h"

#define TOK_BUF_LEN 4

enum DeclaratorKind
{
    DK_VARIABLE,
    DK_PARAMETER,
    DK_FIELD,
    DK_TYPE_DECL,
    DK_IDENT
};

class Parser
{
protected:
    Lexer *_lex;
    SymbolTable _stb;
    AbstractSyntaxTree *_ast;

    TokenKind _sym;
    int _parsingErrors;

    std::map<TokenKind, const char *> _name;

    Token_t _tok, _la;
    Token _tokbuf[TOK_BUF_LEN];
    unsigned _tokIdx;
    unsigned _laIdx;
    unsigned _newLaIdx;

    void getTok();
    TokenKind getLATok(unsigned k);
    const char *getLATokInfoSval(unsigned k);
    void parsingError(const char *msg);
    void check(TokenKind expected);

    void initTokenBuffer();
    virtual void initNames() {}

public:
    virtual void parse(char *output, bool optimize) {}

    AbstractSyntaxTree *getAST()
    {
        return _ast;
    }

    unsigned getCurrentLine()
    {
        return _tok->line;
    }

    Parser(Lexer *lex, char *target) : _lex(lex)
    {
        initNames();
        _parsingErrors = 0;

        _ast = new AbstractSyntaxTree(&_stb, target);
    }

    virtual ~Parser()
    {
        delete _ast;
    }
};

class CParser : public Parser
{
    void initNames();

    bool isTypeSpecifier(TokenKind _kind, int n);
    bool isTypeQualifier(TokenKind kind);
    bool isStorageClassSpecifier(TokenKind kind);

    // THE PARSER RULES

    // Expressions
    ASTNode *PrimaryExpression();
    ASTNode *parsePostfixExpression(ASTNode *expr);
    ASTNode *PostfixExpression(ASTNode *typeName);
    ASTNode *ArgumentExpressionList();
    ASTNode *UnaryExpression(ASTNode *typeName);
    ASTNodeKind UnaryOperator();
    ASTNode *CastExpression();
    ASTNode *MultiplicativeExpression();
    ASTNode *AdditiveExpression();
    ASTNode *ShiftExpression();
    ASTNode *RelationalExpression();
    ASTNode *EqualityExpression();
    ASTNode *AndExpression();
    ASTNode *ExclusiveOrExpression();
    ASTNode *InclusiveOrExpression();
    ASTNode *LogicalAndExpression();
    ASTNode *LogicalOrExpression();
    ASTNode *ConditionalExpression();
    ASTNode *AssignmentExpression();
    ASTNode *Expression();
    ASTNode *ConstantExpression();

    // Declarations
    ASTNode *Declaration(ASTNode *declSpec);
    ASTNode *DeclarationSpecifiers();
    ListASTNode *InitDeclaratorList(ASTNode *typeSpec);
    ASTNode *InitDeclarator(ASTNode *typeSpec);
    void StorageClassSpecifier(unsigned &flags);
    ASTNode *TypeSpecifier();
    ASTNode *StructOrUnionSpecifier();
    ListASTNode *StructDeclarationList();
    ASTNode *StructDeclaration();
    ListASTNode *StructDeclaratorList(ASTNode *typeSpec);
    ASTNode *StructDeclarator(ASTNode *typeSpec);
    ASTNode *EnumSpecifier();
    ListASTNode *EnumeratorList();
    void TypeQualifier(unsigned &flags);
    ASTNode *Declarator(DeclaratorKind, ASTNode *typeSpec);
    ASTNode *DirectDeclarator(DeclaratorKind, ASTNode *typeSpec);
    ASTNode *ParameterTypeList();
    ASTNode *ParameterList();
    ASTNode *ParameterDeclaration();
    ASTNode *IdentifierList();
    ASTNode *TypeName();
    ASTNode *Initializer();
    ASTNode *InitializerList();

    // Statements
    ASTNode *Statement();
    ASTNode *LabeledStatement();
    ASTNode *CompoundStatement();
    ASTNode *FunctionBody();
    ASTNode *DeclarationList();
    ASTNode *StmtOrDeclList();
    ASTNode *SelectionStatement();
    ASTNode *IterationStatement();
    ASTNode *JumpStatement();
    ASTNode *ExpressionStatement();
    ASTNode *TranslationUnit();
    ASTNode *ExternalDeclaration();
    ASTNode *FunctionDefinition(ASTNode *funcType);
public:
    void parse(char *output, bool optimize);

    CParser(CLexer *LEX, char *target) : Parser(LEX, target) {}
};

#endif
