// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 22
// CHECK: 33

int main()
{
    struct
    {
        int x, y;
    } p;

    p.x = 22;
    p.y = 33;

    puti(p.x);
    putc('\n');
    puti(p.y);
    putc('\n');

    return 0;
}
