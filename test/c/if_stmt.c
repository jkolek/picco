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
        z = 33;
    else
        z = 44;

    puti(z);
    putc('\n');
}
