// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 10
// CHECK: 11
// CHECK: 12
// CHECK: 13
// CHECK: 14
// CHECK: 15
// CHECK: 16
// CHECK: 17
// CHECK: 18
// CHECK: 19
// CHECK: 20
// CHECK: 21

int main()
{
    int x;
    int y;

    x = 10;
    y = 22;
    do
    {
        puti(x);
        putc('\n');
        x++;
    } while (x != y);

    return 0;
}
