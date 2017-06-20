// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
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
    int x = 10;
    while (x)
    {
        puti(x);
        putc('\n');
        x--;
    }
    return 0;
}
