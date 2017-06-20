// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 99

int sum(int x, int y)
{
    int a;
    a = x + y;
    return a;
}

int main()
{
    int p, q, r;

    p = 44;
    q = 55;
    r = sum(p, q);
    puti(r);
    putc('\n');
    return 0;
}
