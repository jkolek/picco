// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 2
// CHECK: 6
// CHECK: 10

int main()
{
    int n;

    n = 0;

    do
    {
        n += 2;

        if ((n % 4) == 0)
            continue;

        puti(n);
        putc('\n');
    } while (n < 10);

    return 0;
}
