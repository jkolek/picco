// PICCO Lexer - implementation file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#include "../include/Lexer.h"

#include <cstdio>
#include <ctype.h>
#include <iostream>
#include <map>

void Lexer::error(const char *msg)
{
    std::cerr << "lexer error: " << msg << std::endl;
}

bool Lexer::isHexDigit(char c)
{
    if (c >= 'A' && c <= 'F')
        return true;
    return false;
}

int Lexer::powr(int x, int n)
{
    if (n == 0)
    {
        return 1;
    }
    else if (n == 1)
    {
        return x;
    }
    else
    {
        int res = 1;

        for (unsigned i = 0; i < n; i++)
            res = res * x;

        return res;
    }
}

void Lexer::nextCh()
{
    _ch = getc(_fp);
    _col++;
    if (_ch == '\n')
    {
        _line++;
        _col = 0;
    }
}

Lexer::Lexer(const char *filename)
{
    _line = 1;
    _col = 0;
    _ch = 0;

    // Load source file
    if (filename == NULL)
        _fp = stdin;
    else
        _fp = fopen(filename, "r");

    if (_fp == NULL)
    {
        printf("Fatal error: file '%s' does not exists!\n", filename);
        exit(1);
    }
}

// Search of keyword
TokenKind CLexer::keyword(const char *s)
{
    TokenMap::iterator it = _kwmap.find(s);

    if (it != _kwmap.end())
    {
        return it->second;
    }
    else
    {
        // Search of keyword that can be first in a statement
        it = _fstmtmap.find(s);
        return (it != _fstmtmap.end()) ? (it->second) : TK_IDENT;
    }
}

void CLexer::readName(Token_t t)
{
    int i = 1;

    t->info.sval[0] = _ch;
    nextCh();

    while (isalnum(_ch) || _ch == '_')
    {
        t->info.sval[i++] = _ch;
        nextCh();
    }
    t->info.sval[i] = '\0';

    t->kind = keyword(t->info.sval);
}

void CLexer::readNumber(Token_t t)
{
    char buffer[32];
    int n = 1;
    bool hex = false, real = false;

    t->kind = TK_INT_LIT;
    buffer[0] = _ch;
    nextCh();

    if (_ch == 'x' && buffer[0] == '0')
    {
        nextCh();
        hex = true;
        while (isdigit(_ch) || isHexDigit(_ch))
        {
            buffer[n++] = _ch;
            nextCh();
        }
    }
    else
    {
        while (isdigit(_ch))
        {
            buffer[n++] = _ch;
            nextCh();
        }
    }

    if (_ch == '.')
    {
        real = true;
        nextCh();
    }

    if (hex)
    {
        int currentDigit, result = 0, power = 0;

        n--;
        while (n >= 0)
        {
            if (isdigit(buffer[n]))
                currentDigit = buffer[n] - '0';
            else
                currentDigit = buffer[n] - '7';

            result += currentDigit * powr(16, power);
            power++;
            n--;
        }
        t->info.ival = result;
    }
    else
    {
        int i, digitcnt = 10;

        t->info.ival = buffer[0] - '0';
        for (i = 1; i < n; i++)
            t->info.ival = t->info.ival * 10 + buffer[i] - '0';

        if (real)
        {
            int mantissa = _ch - '0';

            t->kind = TK_FLOAT_LIT;
            nextCh();
            while (isdigit(_ch))
            {
                mantissa = mantissa * 10 + _ch - '0';
                digitcnt *= 10;
                nextCh();
            }
            t->info.fval =
                ((float)t->info.ival) + ((float)mantissa / (float)digitcnt);
        }
    }
}

void CLexer::readString(Token_t t)
{
    int i = 1;

    // Skip the "
    nextCh();
    t->kind = TK_STRING_LIT;
    t->info.sval[0] = _ch;
    nextCh();

    while (_ch != '"')
    {
        t->info.sval[i++] = _ch;
        nextCh();
    }
    t->info.sval[i] = '\0';
    // Skip again the "
    nextCh();
}

void CLexer::readChar(Token_t t)
{
    // Skip the '
    nextCh();
    t->kind = TK_CHAR_LIT;

    if (_ch == '\\')
    {
        nextCh();
        switch (_ch)
        {
            case '0':
                t->info.ival = 0;
                break; // Null char
            case 'b':
                t->info.ival = 8;
                break; // Backspace
            case 't':
                t->info.ival = 9;
                break; // Horizontal tab
            case 'n':
                t->info.ival = 10;
                break; // New line
            case 'v':
                t->info.ival = 11;
                break; // Vertical tab
            case 'f':
                t->info.ival = 12;
                break; // Form feed
            case 'r':
                t->info.ival = 13;
                break; // Carriage return
            case '\'':
                t->info.ival = 39;
                break; // Single quote
            case '\\':
                t->info.ival = 92;
                break; // Backslash
            default:
                error("invalid character constant");
                break;
        }
        nextCh();
    }
    else
    {
        t->info.ival = _ch;
        nextCh();
    }

    if (_ch != '\'')
    {
        printf("line %d, col %d: lexer error: character expected\n", _line, _col);
        // errors++;
        while (_ch != '\'' && _ch != EOF)
        {
            // error
            nextCh();
        }
    }

    t->info.sval[1] = '\0';
    // Skip '
    nextCh();
}

void CLexer::comment()
{
    while (_ch != EOF)
    {
        nextCh();
        if (_ch == '*')
        {
            nextCh();
            if (_ch == '/')
                break;
        }
        else if (_ch == '/')
        {
            nextCh();
            if (_ch == '*')
                comment();
        }
    }
    nextCh();
}

TokenKind CLexer::next(Token_t t)
{
    while (isspace(_ch))
        nextCh();

    t->line = _line;
    t->col = _col;

    if (isalpha(_ch))
    {
        readName(t);
    }
    else if (isdigit(_ch))
    {
        readNumber(t);
    }
    else
    {
        switch (_ch)
        {
            case '"':
                readString(t);
                break;
            case '\'':
                readChar(t);
                break;
            case '&':
                nextCh();
                if (_ch == '&')
                {
                    nextCh();
                    t->kind = TK_LOGICAL_AND;
                }
                else if (_ch == '=')
                {
                    nextCh();
                    t->kind = TK_AND_ASSIGN;
                }
                else
                {
                    t->kind = TK_AND;
                }
                break;
            case '!':
                nextCh();
                if (_ch == '=')
                {
                    nextCh();
                    t->kind = TK_NEQ;
                }
                else
                {
                    t->kind = TK_NOT;
                }
                break;
            case '|':
                nextCh();
                if (_ch == '|')
                {
                    nextCh();
                    t->kind = TK_LOGICAL_OR;
                }
                else if (_ch == '=')
                {
                    nextCh();
                    t->kind = TK_OR_ASSIGN;
                }
                else
                {
                    t->kind = TK_OR;
                }
                break;
            case '{':
                nextCh();
                t->kind = TK_LBRACE;
                break;
            case '}':
                nextCh();
                t->kind = TK_RBRACE;
                break;
            case '(':
                nextCh();
                t->kind = TK_LPAR;
                break;
            case ')':
                nextCh();
                t->kind = TK_RPAR;
                break;
            case '*':
                nextCh();
                if (_ch == '=')
                {
                    nextCh();
                    t->kind = TK_MUL_ASSIGN;
                }
                else
                {
                    t->kind = TK_TIMES;
                }
                break;
            case '+':
                nextCh();
                if (_ch == '+')
                {
                    nextCh();
                    t->kind = TK_INC_OP;
                }
                else if (_ch == '=')
                {
                    nextCh();
                    t->kind = TK_ADD_ASSIGN;
                }
                else
                {
                    t->kind = TK_PLUS;
                }
                break;
            case '-':
                nextCh();
                if (_ch == '-')
                {
                    nextCh();
                    t->kind = TK_DEC_OP;
                }
                else if (_ch == '>')
                {
                    nextCh();
                    t->kind = TK_PTR_OP;
                }
                else if (_ch == '=')
                {
                    nextCh();
                    t->kind = TK_SUB_ASSIGN;
                }
                else
                {
                    t->kind = TK_MINUS;
                }
                break;
            case '%':
                nextCh();
                if (_ch == '=')
                {
                    nextCh();
                    t->kind = TK_MOD_ASSIGN;
                }
                else
                {
                    t->kind = TK_MOD;
                }
                break;
            case ',':
                nextCh();
                t->kind = TK_COMMA;
                break;
            case '/':
                nextCh();
                if (_ch == '/')
                {
                    while (_ch != '\n')
                        nextCh();
                    return next(t);
                }
                else if (_ch == '*')
                {
                    comment();
                    return next(t);
                }
                else if (_ch == '=')
                {
                    nextCh();
                    t->kind = TK_DIV_ASSIGN;
                }
                else
                {
                    t->kind = TK_DIV;
                }
                break;
            case ';':
                nextCh();
                t->kind = TK_SEMICOLON;
                break;
            case '=':
                nextCh();
                if (_ch == '=')
                {
                    nextCh();
                    t->kind = TK_EQL;
                }
                else
                {
                    t->kind = TK_ASSIGN;
                }
                break;
            case '[':
                nextCh();
                t->kind = TK_LBRACK;
                break;
            case ']':
                nextCh();
                t->kind = TK_RBRACK;
                break;
            case '^':
                nextCh();
                if (_ch == '=')
                {
                    nextCh();
                    t->kind = TK_XOR_ASSIGN;
                }
                else
                {
                    t->kind = TK_EXCLUSIVE_OR;
                }
                break;
            case '~':
                nextCh();
                t->kind = TK_TILDA;
                break;
            case '?':
                nextCh();
                t->kind = TK_COND_OP;
                break;
            case ':':
                nextCh();
                t->kind = TK_COLON;
                break;
            case '<':
                nextCh();
                if (_ch == '=')
                {
                    nextCh();
                    t->kind = TK_LEQ;
                }
                else if (_ch == '<')
                {
                    nextCh();
                    if (_ch == '=')
                    {
                        nextCh();
                        t->kind = TK_LSHIFT_ASSIGN;
                    }
                    else
                    {
                        t->kind = TK_LSHIFT_OP;
                    }
                }
                else
                {
                    t->kind = TK_LSS;
                }
                break;
            case '>':
                nextCh();
                if (_ch == '=')
                {
                    nextCh();
                    t->kind = TK_GEQ;
                }
                else if (_ch == '>')
                {
                    nextCh();
                    if (_ch == '=')
                    {
                        nextCh();
                        t->kind = TK_RSHIFT_ASSIGN;
                    }
                    else
                    {
                        t->kind = TK_RSHIFT_OP;
                    }
                }
                else
                {
                    t->kind = TK_GTR;
                }
                break;
            case '.':
                nextCh();
                if (_ch == '.')
                {
                    nextCh();
                    if (_ch == '.')
                    {
                        nextCh();
                        t->kind = TK_PERIODS;
                    }
                    else
                    {
                        t->kind = TK_UNKNOWN;
                    }
                }
                else
                    t->kind = TK_PERIOD;
                break;
            case EOF:
                t->kind = TK_EOF;
                break;
            default:
                nextCh();
                t->kind = TK_UNKNOWN;
                break;
        }
    }

    return t->kind;
}

void CLexer::initialize()
{
    // Insert reserved words
    INSERT_KEYWORD("auto", TK_AUTO);
    INSERT_KEYWORD("char", TK_CHAR);
    INSERT_KEYWORD("const", TK_CONST);
    INSERT_KEYWORD("double", TK_DOUBLE);
    INSERT_KEYWORD("else", TK_ELSE);
    INSERT_KEYWORD("enum", TK_ENUM);
    INSERT_KEYWORD("extern", TK_EXTERN);
    INSERT_KEYWORD("float", TK_FLOAT);
    INSERT_KEYWORD("int", TK_INT);
    INSERT_KEYWORD("long", TK_LONG);
    INSERT_KEYWORD("register", TK_REGISTER);
    INSERT_KEYWORD("short", TK_SHORT);
    INSERT_KEYWORD("signed", TK_SIGNED);
    INSERT_KEYWORD("sizeof", TK_SIZEOF);
    INSERT_KEYWORD("static", TK_STATIC);
    INSERT_KEYWORD("struct", TK_STRUCT);
    INSERT_KEYWORD("typedef", TK_TYPEDEF);
    INSERT_KEYWORD("union", TK_UNION);
    INSERT_KEYWORD("unsigned", TK_UNSIGNED);
    INSERT_KEYWORD("void", TK_VOID);
    INSERT_KEYWORD("volatile", TK_VOLATILE);

    INSERT_FIRST_OF_STMT_KEYWORD("asm", TK_ASM);
    INSERT_FIRST_OF_STMT_KEYWORD("break", TK_BREAK);
    INSERT_FIRST_OF_STMT_KEYWORD("case", TK_CASE);
    INSERT_FIRST_OF_STMT_KEYWORD("continue", TK_CONTINUE);
    INSERT_FIRST_OF_STMT_KEYWORD("default", TK_DEFAULT);
    INSERT_FIRST_OF_STMT_KEYWORD("do", TK_DO);
    INSERT_FIRST_OF_STMT_KEYWORD("for", TK_FOR);
    INSERT_FIRST_OF_STMT_KEYWORD("goto", TK_GOTO);
    INSERT_FIRST_OF_STMT_KEYWORD("if", TK_IF);
    INSERT_FIRST_OF_STMT_KEYWORD("putc", TK_PUTC);
    INSERT_FIRST_OF_STMT_KEYWORD("puti", TK_PUTI);
    INSERT_FIRST_OF_STMT_KEYWORD("return", TK_RETURN);
    INSERT_FIRST_OF_STMT_KEYWORD("switch", TK_SWITCH);
    INSERT_FIRST_OF_STMT_KEYWORD("while", TK_WHILE);
}
