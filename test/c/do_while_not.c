// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 1

int main()
{
    int n, ok;

    n = 0;
    ok = 0;

    do
    {
        n += 2;
        if (n > 10)
            ok = 1;
    } while (!ok);

    puti(ok);
    putc('\n');

    return 0;
}
