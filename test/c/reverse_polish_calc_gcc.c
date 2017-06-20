// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 45
// CHECK: 50
// CHECK: +
// CHECK: 95
// CHECK: 10
// CHECK: *
// CHECK: 950
// CHECK: 20
// CHECK: +
// CHECK: 970

/* Reverse Polish Calculator */

int puts(char *str);

enum TokenKind
{
    TK_UNKNOWN,
    TK_NUMBER,
    TK_PLUS,
    TK_MINUS,
    TK_TIMES,
    TK_DIV,
    TK_EOF
};

/* Character buffer */
char buf[100];
int idx;

/* Stack and corresponding functions */
int stack[100];
int sp;

void push(int val)
{
    stack[sp] = val;
    sp++;
}

int pop()
{
    sp--;
    return stack[sp];
}

/* Parsing */

int tok_ival;

int isspace(char ch)
{
    if (ch == ' ' || ch == '\n')
        return 1;
    return 0;
}

int isnumber(char ch)
{
    if (ch >= '0' && ch <= '9')
        return 1;
    return 0;
}

char nextCh()
{
    char ch = buf[idx];
    idx++;
    return ch;
}

int getTok()
{
    int tok;
    char ch;

    ch = nextCh();
    while (isspace(ch))
        ch = nextCh();

    if (ch == '$')
        tok = TK_EOF;
    else if (isnumber(ch))
    {
        tok = TK_NUMBER;
        tok_ival = ch - '0';
        ch = nextCh();
        while (isnumber(ch))
        {
            tok_ival = (tok_ival * 10) + (ch - '0');
            ch = nextCh();
        }
    }
    else if (ch == '+')
        tok = TK_PLUS;
    else if (ch == '-')
        tok = TK_MINUS;
    else if (ch == '*')
        tok = TK_TIMES;
    else if (ch == '/')
        tok = TK_DIV;
    else
        tok = TK_UNKNOWN;

    return tok;
}

/* Initialize the environment */
void init()
{
    sp = 0;
    /*   45 50 + 10 * 20 +   */
    buf[0] = '4';
    buf[1] = '5';
    buf[2] = ' ';
    buf[3] = '5';
    buf[4] = '0';
    buf[5] = ' ';
    buf[6] = '+';
    buf[7] = ' ';
    buf[8] = '1';
    buf[9] = '0';
    buf[10] = ' ';
    buf[11] = '*';
    buf[12] = ' ';
    buf[13] = '2';
    buf[14] = '0';
    buf[15] = ' ';
    buf[16] = '+';
    buf[17] = '$'; /* END mark */
    idx = 0;
}

void putcln(char ch)
{
    putc(ch);
    putc('\n');
}

void putiln(int val)
{
    puti(val);
    putc('\n');
}

int main()
{
    int sym, val;

    puts("Starting ...");

    init();

    sym = getTok();
    while (sym != TK_EOF)
    {
        if (sym == TK_NUMBER)
        {
            push(tok_ival);
            putiln(tok_ival);
        }
        else if (sym == TK_PLUS)
        {
            val = pop() + pop();
            push(val);
            putcln('+');
            putiln(val);
            /*
            BUG: Unexpected output if enable this.
            }
            else if (sym == TK_MINUS)
            {
              val = pop();
              val = pop() - val;
              push(val);
              putcln('-');
              putiln(val);*/
        }
        else if (sym == TK_TIMES)
        {
            val = pop() * pop();
            push(val);
            putcln('*');
            putiln(val);
        }
        else if (sym == TK_DIV)
        {
            val = pop();
            if (val != 0)
            {
                val = pop() / val;
                push(val);
                putcln('/');
                putiln(val);
            }
            else
            {
                putcln('e');
            }
        }
        sym = getTok();
    }

    return 0;
}
