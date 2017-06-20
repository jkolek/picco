// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: -5

int main()
{
    unsigned i = 4, x;

    x = ~i;

    puti(x);
    putc('\n');

    return 0;
}
