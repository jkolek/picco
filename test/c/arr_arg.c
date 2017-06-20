// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 21
// CHECK: 22
// CHECK: 23
// CHECK: 24
// CHECK: 25
// CHECK: 0
// CHECK: 0
// CHECK: 0
// CHECK: 0
// CHECK: 0

void print_arr(int *a, int len)
{
    int n;
    n = 0;
    while (n < len)
    {
        puti(a[n]);
        putc('\n');
        n++;
    }
}

int main()
{
    int a[10];

    a[0] = 21;
    a[1] = 22;
    a[2] = 23;
    a[3] = 24;
    a[4] = 25;

    print_arr(a, 10);
    return 0;
}
