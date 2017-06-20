// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 2
// CHECK: 4
// CHECK: 6
// CHECK: 8
// CHECK: 10
// CHECK: 12

int main()
{
    int n;

    n = 0;

    while (1)
    {
        n += 2;

        puti(n);
        putc('\n');

        if (n > 10)
            break;
    }

    return 0;
}
