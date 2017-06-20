// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 4
// CHECK: 4

int main()
{
    int x, y, z, immDiv2;
    y = 9;
    z = 2;
    x = y / z;
    immDiv2 = y / 2;

    puti(x);
    putc('\n');

    puti(immDiv2);
    putc('\n');
    return 0;
}
