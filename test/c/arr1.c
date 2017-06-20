// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 33
// CHECK: 22
// CHECK: 4
// CHECK: 5
// CHECK: 2
// CHECK: 10
// CHECK: 12
// CHECK: 11
// CHECK: 20
// CHECK: 15
// CHECK: o: 4
// CHECK: e: 6

int main()
{
    int a[10];
    int i, j, odd, even;

    a[0] = 33;
    a[1] = 22;
    a[2] = 4;
    a[3] = 5;
    a[4] = 2;
    a[5] = 10;
    a[6] = 12;
    a[7] = 11;
    a[8] = 20;
    a[9] = 15;

    i = 0;
    while (i < 10)
    {
        puti(a[i]);
        putc('\n');
        i++;
    }

    i = 0;
    odd = 0;
    even = 0;
    while (i < 10)
    {
        if (a[i] % 2 == 0)
            even++;
        else
            odd++;
        i++;
    }

    putc('o');
    putc(':');
    putc(' ');
    puti(odd);
    putc('\n');

    putc('e');
    putc(':');
    putc(' ');
    puti(even);
    putc('\n');

    return 0;
}
