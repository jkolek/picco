// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 31
// CHECK: 125

int main()
{
    unsigned n, k;
    int i, j;

    k = 1000;
    j = 1000;

    n = k >> 5;
    i = j >> 3;

    puti(n);
    putc('\n');
    puti(i);
    putc('\n');

    return 0;
}
