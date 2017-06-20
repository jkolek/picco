// PICCO asm parser - header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef ASM_PARSER_H
#define ASM_PARSER_H

#include "Lexer.h"
#include <ctype.h>

#define TOK_BUF_LEN 4

enum AsmTokenKind
{
    ASM_TK_IDENT,
    ASM_TK_DIRECTIVE,
    ASM_TK_INSTR,
    ASM_TK_REGISTER,
    ASM_TK_INTEGER_CONST,
    ASM_TK_COMMA,
    ASM_TK_COLON,
    ASM_TK_SEMICOLON,
    ASM_TK_PLUS,
    ASM_TK_MINUS,
    ASM_TK_TIMES,
    ASM_TK_DIV,
    ASM_TK_MOD,
    ASM_TK_LPAR,
    ASM_TK_RPAR,
    ASM_TK_PERIOD,
    ASM_TK_DOLLAR,
    ASM_TK_UNKNOWN,
    ASM_TK_EOF
};

// Asm token type
typedef struct asm_token_t
{
    AsmTokenKind kind;
    union
    {
        char sval[MAXSTR];
        int ival;
        float fval;
    } info;
    unsigned instrKind; // This is target specific
    unsigned line, col;
} * AsmToken_t;

class AsmParser
{
protected:
    FILE *fp;
    char ch;
    unsigned line, col;

    void error(const char *msg);
    void initTokenBuffer();
    void nextCh();
    void readIdent(AsmToken_t t);
    void readNumber(AsmToken_t t);
    AsmTokenKind next(AsmToken_t t);

    virtual AsmTokenKind instr(const char *name, AsmToken_t t)
    {
        return ASM_TK_UNKNOWN;
    }

    AsmTokenKind sym;
    AsmToken_t tok, la;
    AsmToken_t tokbuf[TOK_BUF_LEN];
    unsigned tokIdx;
    unsigned laIdx;
    unsigned newLaIdx;
    char msg[MAXSTR];
    int syntaxErrors;

    void getTok();
    void check(AsmTokenKind expected);
    virtual unsigned parseRegister() { return 0x0; }
    virtual int parseIntegerConst() { return -1; }
    virtual void parseStatement() {}
    virtual void parseLabel() {}
    virtual void parseDirective() {}

public:
    void parse(const char *filename);

    AsmParser()
    {
        line = 1;
        col = 0;
    }

    virtual ~AsmParser() {}
};

#endif
