// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 55

int main()
{
    int a[5];
    int x, y, z;

    a[0] = 22;
    a[1] = 33;
    x = a[0];
    y = a[1];
    z = x + y;
    puti(z);
    putc('\n');
}
