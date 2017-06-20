// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: -1

int main()
{
    int x, y, z, p;

    x = (y & 10) | (z ^ 8) | ~p;

    puti(x);
    putc('\n');

    return 0;
}
