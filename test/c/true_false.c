// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 33

int main()
{
    int x;

    if (0)
    {
        x = 22;
    }
    else
    {
        x = 33;
    }

    puti(x);
    putc('\n');

    return 0;
}
