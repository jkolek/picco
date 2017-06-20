// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 100

int main()
{
    int x, y, z;

    x = 10;
    y = 0;
    if (x == 0 || y == 0)
    {
        z = 100;
    }
    puti(z);
    putc('\n');
    return 0;
}
