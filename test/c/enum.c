// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 1
// CHECK: 2
// CHECK: 4

enum TokenKind
{
    TK_IDENT,
    TK_NUMBER,
    TK_PLUS,
    TK_MINUS,
    TK_TIMES,
    TK_DIV
};

void main()
{
    int x, y, z;

    x = TK_NUMBER;
    y = TK_PLUS;
    z = TK_TIMES;

    puti(x);
    putc('\n');
    puti(y);
    putc('\n');
    puti(z);
    putc('\n');
}
