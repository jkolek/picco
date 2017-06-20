// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 0
// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 4
// CHECK: 5
// CHECK: 20
// CHECK: 19
// CHECK: 18
// CHECK: 17
// CHECK: 16
// CHECK: 15
// CHECK: 14

int main()
{
    int x, y, z;

    x = 0;

    while (x < 10)
    {
        puti(x);
        putc('\n');
        if (x == 5)
            break;
        x++;
    }

    x = 20;

    do
    {
        puti(x);
        putc('\n');
        if (x == 14)
            break;
        x--;
    } while (x > 10);

    return 0;
}
