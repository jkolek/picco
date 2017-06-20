// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 123
// CHECK: 105

void out(int i)
{
    puti(i);
    putc('\n');
}

void main()
{
    int p, q, r;

    p = 22;
    r = 22;

    if (p <= r)
    {
        q = 123;
    }
    else
    {
        q = 321;
    }

    out(q);
    r = 345;
    p = 1;

    if (p >= r)
    {
        q = 99;
    }
    else
    {
        q = 105;
    }

    out(q);
}
