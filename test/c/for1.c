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
// CHECK: 10

int main()
{
    int x;

    x = 0;

    for (;;)
    {
        x++;
        puti(x);
        putc('\n');
        if (x == 10)
            break;
    }

    return 0;
}
