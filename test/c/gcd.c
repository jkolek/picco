// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 3

void main()
{
    int a, b;

    a = 9;
    b = 15;
    while (a != b)
    {
        if (a > b)
            a = a - b;
        else
            b = b - a;
    }
    puti(a);
    putc('\n');
}
