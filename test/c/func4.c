// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 30

int sum(int x, int y) { return x + y; }

void main()
{
    int a, b;

    b = 20;
    a = sum(10, b);
    puti(a);
    putc('\n');
}
