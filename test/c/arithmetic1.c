// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: -114

int main()
{
    int x;
    int y;
    int z;

    x = 11;
    y = 12;

    z = (((x * 33) + (y * 44)) / 100) - 122;

    puti(z);
    putc('\n');

    return 0;
}
