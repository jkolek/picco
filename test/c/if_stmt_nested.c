// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 22

void main()
{
    int x;
    int y;
    int z;

    x = 10;
    y = 20;

    if (x != y)
    {
        if (x == 10)
            z = 22;
        else
            z = 33;
    }
    else
    {
        if (y == 10)
            z = 44;
        else
            z = 55;
    }

    puti(z);
    putc('\n');
}
