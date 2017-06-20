// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 50000

int add(int x, int y) { return x + y; }

int sub(int x, int y) { return x - y; }

int mul(int x, int y) { return x * y; }

void out(int x)
{
    puti(x);
    putc('\n');
}

int main()
{
    int p, q, r;

    p = 300;
    q = 200;
    r = mul(add(p, q), sub(p, q));
    out(r);
    return 0;
}
