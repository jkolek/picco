// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 977

int add(int x, int y, int a, int b) { return x + y + a + b; }

int main()
{
    int x, y, z, a, b;
    int p, q, r, s;

    x = 11;
    y = 22;
    a = 33;
    b = 44;
    if (y == 200)
    {
        a = 1;
        b = 2;
        z = 55;
    }
    else
    {
        p = 400;
        q = 500;
        z = add(p, b, a, q);
    }

    puti(z);
    putc('\n');

    return 0;
}
