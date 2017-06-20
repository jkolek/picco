// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 33

int a;

int main()
{
    int i;
    int *p;
    int x;

    i = 33;
    p = &i;
    //*p = 10;
    x = *p;

    puti(i);
    putc('\n');

    puti(*p);
    putc('\n');

    puti(x);
    putc('\n');

    return 0;
}
