// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 5
// CHECK: 6
// CHECK: 7
// CHECK: 8
// CHECK: 9
// CHECK: 10
// CHECK: 14
// CHECK: 13
// CHECK: 12
// CHECK: 11
// CHECK: 10

int main()
{
    int x, y, z;

    x = 0;

    while (x < 10)
    {
        x++;
        if (x < 5)
            continue;
        puti(x);
        putc('\n');
    }

    x = 20;

    do
    {
        x--;
        if (x > 14)
            continue;
        puti(x);
        putc('\n');
    } while (x > 10);

    return 0;
}
