// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 43

int sum(int x, int y)
{
    int a, b;
    a = 333;
    b = 444;
    return x + y;
}

void main()
{
    int x;
    int y;
    int z;

    x = 10;
    y = 20;

    if (x == y)
        z = sum(x, y);
    else
        z = sum(x, 33);
    puti(z);
    putc('\n');
}
