// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 33

void main()
{
    int x;
    int y;
    int z;

    x = 10;
    y = 20;

    if (x != y)
    {
        int a, b;
        b = 33;
        z = b;
    }
    else
    {
        int p, q;
        q = 102;
        z = 44;
    }

    puti(z);
    putc('\n');
}
