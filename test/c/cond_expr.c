// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 20
// CHECK: 1

int main()
{
    int x, y, z;

    y = 1;
    z = 2;

    x = (y > z) ? 10 : 20;

    puti(x);
    putc('\n');

    x = (y < z) ? y : z;

    puti(x);
    putc('\n');

    return 0;
}
