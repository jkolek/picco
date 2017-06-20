// PICCO asm parser - header file.
// Copyright (C) 2017  Jozef Kolek <jkolek@gmail.com>
//
// All rights reserved.
//
// See the LICENSE file for more details.

#include "../include/AsmParser.h"

void AsmParser::error(const char *msg)
{
    printf("line: %d; col: %d; error: %s\n", tok->line, tok->col, msg);
    exit(1);
}

void AsmParser::initTokenBuffer()
{
    for (unsigned i = 0; i < TOK_BUF_LEN; i++)
        tokbuf[i] = (AsmToken_t)malloc(sizeof(asm_token_t));

    sym = next(tokbuf[0]);
    next(tokbuf[1]);
    next(tokbuf[2]);
    laIdx = 0;
}

void AsmParser::nextCh()
{
    ch = getc(fp);
    col++;
    if (ch == '\n')
    {
        line++;
        col = 0;
    }
}

void AsmParser::readIdent(AsmToken_t t)
{
    int i;

    t->info.sval[0] = ch;
    nextCh();

    i = 1;
    while (isalnum(ch) || ch == '_')
    {
        t->info.sval[i++] = ch;
        nextCh();
    }
    t->info.sval[i] = '\0';
    t->kind = instr(t->info.sval, t);
}

static int powr(int x, int n)
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

void AsmParser::readNumber(AsmToken_t t)
{
    char buffer[32];
    int n = 1;
    bool hex = false;
    bool real = false;

    t->kind = ASM_TK_INTEGER_CONST;
    buffer[0] = ch;
    nextCh();

    if (ch == 'x' && buffer[0] == '0')
    {
        nextCh();
        hex = true;
        while (isdigit(ch) || (ch >= 'a' && ch <= 'f') ||
               (ch >= 'A' && ch <= 'F'))
        {
            buffer[n++] = ch;
            nextCh();
        }
    }
    else
    {
        while (isdigit(ch))
        {
            buffer[n++] = ch;
            nextCh();
        }
    }

    if (ch == '.')
    {
        real = true;
        nextCh();
    }

    if (hex)
    {
        int currentDigit;
        int result = 0;
        int power = 0;

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
        int i;
        int digitcnt = 10;

        t->info.ival = buffer[0] - '0';
        for (i = 1; i < n; i++)
            t->info.ival = t->info.ival * 10 + buffer[i] - '0';

        /*if (real)
        {
            t->kind = TK_FLOAT_LIT;
            int mantissa = ch - '0';
            nextCh();
            while (isdigit(ch))
            {
                mantissa = mantissa * 10 + ch - '0';
                digitcnt *= 10;
                nextCh();
            }
            t->info.fval = ((float) t->info.ival) +
                            ((float) mantissa / (float) digitcnt);
        }*/
    }
}

AsmTokenKind AsmParser::next(AsmToken_t t)
{
    while (isspace(ch))
        nextCh();

    t->line = line;
    t->col = col;

    if (isalpha(ch))
        readIdent(t);
    else if (isdigit(ch))
        readNumber(t);
    else
    {
        switch (ch)
        {
            // case '"': readString(t); break;
            // case '\'': readChar(t); break;
            case '(':
                nextCh();
                t->kind = ASM_TK_LPAR;
                break;
            case ')':
                nextCh();
                t->kind = ASM_TK_RPAR;
                break;
            case '*':
                nextCh();
                t->kind = ASM_TK_TIMES;
                break;
            case '+':
                nextCh();
                t->kind = ASM_TK_PLUS;
                break;
            case '-':
                nextCh();
                t->kind = ASM_TK_MINUS;
                break;
            case '%':
                nextCh();
                t->kind = ASM_TK_MOD;
                break;
            case ',':
                nextCh();
                t->kind = ASM_TK_COMMA;
                break;
            case '/':
                nextCh();
                t->kind = ASM_TK_DIV;
                break;
            case ';':
                nextCh();
                t->kind = ASM_TK_SEMICOLON;
                break;
            case ':':
                nextCh();
                t->kind = ASM_TK_COLON;
                break;
            case '.':
                nextCh();
                t->kind = ASM_TK_PERIOD;
                break;
            case '$':
                nextCh();
                t->kind = ASM_TK_DOLLAR;
                break;
            case '#':
                while (ch != '\n')
                    nextCh();
                return next(t);
            case EOF:
                t->kind = ASM_TK_EOF;
                break;
            default:
                nextCh();
                t->kind = ASM_TK_UNKNOWN;
                break;
        }
    }
    return t->kind;
}

void AsmParser::getTok()
{
    tokIdx = laIdx;
    laIdx = (laIdx + 1) % TOK_BUF_LEN;
    newLaIdx = (laIdx + 2) % TOK_BUF_LEN;

    // Get the current token
    tok = tokbuf[tokIdx];

    // Get symbol type of first lookahead token
    sym = tokbuf[laIdx]->kind;

    // Allocate a new third lookahead token
    next(tokbuf[newLaIdx]);
}

void AsmParser::check(AsmTokenKind expected)
{
    if (sym == expected)
        getTok();
    else
        error("invalid token");
}

void AsmParser::parse(const char *filename)
{
    // Load input file
    if (filename == NULL)
        fp = stdin;
    else
        fp = fopen(filename, "r");

    if (fp == NULL)
    {
        printf("Fatal error: file '%s' does not exists!\n", filename);
        exit(1);
    }

    nextCh();
    initTokenBuffer();

    for (;;)
    {
        if (sym == ASM_TK_INSTR)
        {
            parseStatement();
        }
        else if (sym == ASM_TK_IDENT)
        {
            parseLabel();
        }
        else if (sym == ASM_TK_PERIOD)
        {
            getTok();
            parseDirective();
        }
        else
            break;
    }
}
