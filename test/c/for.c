// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 0
// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 4
// CHECK: 5
// CHECK: 6
// CHECK: 7
// CHECK: 8
// CHECK: 9
// CHECK:
// CHECK: 10
// CHECK: 9
// CHECK: 8
// CHECK: 7
// CHECK: 6
// CHECK: 5
// CHECK: 4
// CHECK: 3
// CHECK: 2
// CHECK: 1

int main()
{
    int i;

    for (i = 0; i < 10; i++)
    {
        puti(i);
        putc('\n');
    }

    putc('\n');

    for (i = 10; i > 0; i--)
    {
        puti(i);
        putc('\n');
    }

    return 0;
}
