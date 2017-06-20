// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 22

int main()
{
    int x, y, z;
    x = 70;
    y = 60;
    if (x > y)
        z = 22;
    else
        z = 33;
    puti(z);
    putc('\n');
    return 0;
}
