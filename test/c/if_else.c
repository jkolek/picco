// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 44

int main()
{
    int x, y, z;
    x = 10;
    y = 20;

    if (x != 10)
    {
        z = 33;
    }
    else
    {
        z = 44;
    }

    puti(z);
    putc('\n');

    return 0;
}
