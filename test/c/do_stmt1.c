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

int main()
{
    int x = 0;

    do
    {
        puti(x);
        putc('\n');
        x++;
    } while (x < 8);

    return x;
}
