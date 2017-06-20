// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 16
// CHECK: 48
// CHECK: 16

int main()
{
    int a, b, i, n;

    a = 2;

    n = 3;
    i = 0;

    // shift a value in variable by an immediate
    while (i < n)
    {
        a = a << 1;
        i++;
    }

    puti(a);
    putc('\n');

    b = 4;
    a = 3 << b;

    puti(a);
    putc('\n');

    n = 3;
    i = 1;
    a = 128;

    do
    {
        a = a >> i;
        i++;
    } while (i < n);

    puti(a);
    putc('\n');

    return a;
}
