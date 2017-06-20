// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 1

int main()
{
    int n, ok;

    n = 0;
    ok = 0;

    if (!ok)
    {
        ok = 1;
    }

    puti(ok);
    putc('\n');

    return 0;
}
