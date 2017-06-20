// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 10

int main()
{
    int x = 10;

    if (x)
    {
        puti(x);
        putc('\n');
    }

    return 0;
}
