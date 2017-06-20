// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 1
// CHECK: 32
// CHECK: 33

int main()
{
    int a[10];
    int i, j;

    i = 0;
    a[i++] = 32;
    a[i++] = 33;

    j = 5;

    a[++j] = 55;
    a[++j] = 56;

    /*a[1] = 1;
    a[2] = 3;
    a[3] = 2;
    a[4] = 22;
    a[5] = 15;
    a[6] = 10;
    a[7] = 14;
    a[8] = 30;
    a[9] = 5;*/

    puti(i);
    putc('\n');

    puti(a[0]);
    putc('\n');

    puti(a[1]);
    putc('\n');

    puti(a[6]);
    putc('\n');

    puti(a[7]);
    putc('\n');

    return 0;
}
