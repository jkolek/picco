// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 22
// CHECK: 55

int a;

int main()
{
    int x, y, z;

    y = 33;

    z = (x = 22) + y;

    puti(x);
    putc('\n');
    puti(z);
    putc('\n');

    return 0;
}
