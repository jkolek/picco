// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 15
// CHECK: 16
// CHECK: 17

int main()
{
    short a[10];

    a[0] = 15;
    a[1] = 16;
    a[2] = 17;

    puti(a[0]);
    putc('\n');
    puti(a[1]);
    putc('\n');
    puti(a[2]);
    putc('\n');

    return 0;
}
