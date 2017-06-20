// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 30

int main()
{
    int a = 10, b = 20;
    int x;

    x = a + b;

    puti(x);
    putc('\n');

    return 0;
}
