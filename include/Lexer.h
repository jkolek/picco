// PICCO Lexer - header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#ifndef LEXER_H
#define LEXER_H

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "common.h"
#include <map>

// Token Kinds

enum TokenKind
{
    TK_UNKNOWN,
    TK_IDENT,
    TK_INT_LIT,
    TK_CHAR_LIT,
    TK_FLOAT_LIT,
    TK_STRING_LIT,

    TK_SIZEOF,

    TK_PLUS,
    TK_MINUS,
    TK_TIMES,
    TK_DIV,
    TK_MOD,

    TK_PERIODS,
    TK_SEMICOLON,
    TK_COLON,
    TK_COMMA,
    TK_PERIOD,
    TK_LPAR,
    TK_RPAR,
    TK_LBRACK,
    TK_RBRACK,
    TK_LBRACE,
    TK_RBRACE,

    TK_TILDA,   // ~
    TK_COND_OP, // ?

    TK_AND,          // &
    TK_LOGICAL_AND,  // &&
    TK_OR,           // |
    TK_LOGICAL_OR,   // ||
    TK_EXCLUSIVE_OR, // ^

    TK_PTR_OP,
    TK_INC_OP,
    TK_DEC_OP,
    TK_LSHIFT_OP,
    TK_RSHIFT_OP,

    TK_LSS,
    TK_GTR,
    TK_LEQ,
    TK_GEQ,
    TK_EQL,
    TK_NEQ,
    TK_NOT,

    TK_ASSIGN,
    TK_MUL_ASSIGN, // *=
    TK_DIV_ASSIGN,
    TK_MOD_ASSIGN,
    TK_ADD_ASSIGN,
    TK_SUB_ASSIGN,
    TK_LSHIFT_ASSIGN,
    TK_RSHIFT_ASSIGN,
    TK_AND_ASSIGN,
    TK_XOR_ASSIGN,
    TK_OR_ASSIGN,

    TK_TYPE_NAME,
    TK_ELLIPSIS, // ...

    // FirstOf Declaration
    TK_AUTO,
    TK_CHAR,
    TK_CONST,
    TK_DOUBLE,
    TK_ELSE,
    TK_ENUM,
    TK_EXTERN,
    TK_FLOAT,
    TK_INT,
    TK_LONG,
    TK_REGISTER,
    TK_SHORT,
    TK_SIGNED,
    TK_STATIC,
    TK_STRUCT,
    TK_TYPEDEF,
    TK_UNION,
    TK_UNSIGNED,
    TK_VOID,
    TK_VOLATILE,

    // FirstOf Statement
    TK_ASM,
    TK_BREAK,
    TK_CASE,
    TK_CONTINUE,
    TK_DEFAULT,
    TK_DO,
    TK_FOR,
    TK_GOTO,
    TK_IF,
    TK_PUTC,
    TK_PUTI,
    TK_RETURN,
    TK_SWITCH,
    TK_WHILE,

    TK_EOF
};

// Token type
typedef struct Token
{
    TokenKind kind;
    union
    {
        char sval[MAXSTR];
        int ival;
        float fval;
    } info;
    int line, col;
} *Token_t;

typedef std::map<const char *, TokenKind, CharCompare> TokenMap;

class Lexer
{
protected:
    FILE *_fp;
    int _ch;   // Current character
    int _line; // Current line
    int _col;  // Current column

    void error(const char *msg);
    bool isHexDigit(char c);
    int powr(int x, int n);

    virtual void readName(Token_t t) {}
    virtual void readNumber(Token_t t) {}
    virtual void readString(Token_t t) {}
    virtual void readChar(Token_t t) {}
    virtual void comment() {}

public:
    void nextCh();
    virtual TokenKind next(Token_t t) { return TK_UNKNOWN; }

    Lexer(const char *filename);

    ~Lexer() {}
};

#define INSERT_KEYWORD(keyword, kind)                                          \
    _kwmap.insert(TokenMap::value_type(keyword, kind))

#define INSERT_FIRST_OF_STMT_KEYWORD(keyword, kind)                            \
    _fstmtmap.insert(TokenMap::value_type(keyword, kind))

class CLexer : public Lexer
{
    TokenMap _kwmap;
    TokenMap _fstmtmap;

    TokenKind keyword(const char *s);
    void readName(Token_t t);
    void readNumber(Token_t t);
    void readString(Token_t t);
    void readChar(Token_t t);
    void comment();
    void initialize();

public:
    TokenKind next(Token_t t);

    CLexer(const char *filename) : Lexer(filename)
    {
        initialize();
    }
};

#endif
