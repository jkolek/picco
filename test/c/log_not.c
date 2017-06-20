// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 321
// CHECK: 400

int main()
{
    int x, y, z;

    x = 1;
    if (!x)
        y = 123;
    else
        y = 321;

    x = 0;
    if (!x)
        z = 400;
    else
        z = 500;

    puti(y);
    putc('\n');
    puti(z);
    putc('\n');

    return 0;
}
