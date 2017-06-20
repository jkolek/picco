// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 25

int sum(int x, int y)
{
    int result;

    result = x + y;
    return result;
}

int main()
{
    int x;
    int y;
    int res;
    int z, s;

    x = 5;
    y = 20;

    if (x != y)
    {
        z = sum(x, y);
        res = z;
    }
    else
    {
        s = sum(x, 33);
        res = s;
    }

    puti(res);
    putc('\n');

    return res;
}
