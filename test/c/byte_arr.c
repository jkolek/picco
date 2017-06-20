// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: a
// CHECK: b
// CHECK: c

int main()
{
    char a[10];

    a[0] = 'a';
    a[1] = 'b';
    a[2] = 'c';

    putc(a[0]);
    putc('\n');
    putc(a[1]);
    putc('\n');
    putc(a[2]);
    putc('\n');

    return 0;
}
