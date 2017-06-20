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

void main()
{
    int x;

    x = 0;

    while (x != 10)
    {
        puti(x);
        putc('\n');
        x++;
    }
}
