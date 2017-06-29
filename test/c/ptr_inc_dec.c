// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 9
// CHECK: 7
// CHECK: 6

int main()
{
    int *x = 5;
    short *y = 5;
    char *z = 5;

    x++;
    y++;
    z++;

    puti(x);
    putc('\n');

    puti(y);
    putc('\n');

    puti(z);
    putc('\n');

    return 0;
}
