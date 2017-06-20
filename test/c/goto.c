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
// CHECK: 55
// CHECK: 10

int sum(int x, int y) { return x + y; }

void main()
{
    int x;
    int y;
    int z;
    int i;

    x = 33;
    y = 22;

    goto saberi;
    x = 10;
    y = 20;

saberi:
    z = x + y;

    i = 0;

loop:
    if (i < 10)
    {
        puti(i);
        putc('\n');
        i++;
        goto loop;
    }

    putc('\n');

    puti(z);
    putc('\n');
    puti(i);
    putc('\n');
}
