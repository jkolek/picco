// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 1230

int a(int x, int y)
{
    int k;

    k = x * y;

    return k;
}

int b(int x, int y)
{
    int k, n;

    n = a(30, 40);
    k = x + y + n;

    return k;
}

int main()
{
    int x;

    x = b(10, 20);

    // Expected x == 1230
    puti(x);
    putc('\n');

    return 0;
}
