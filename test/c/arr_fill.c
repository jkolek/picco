// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// FIXME: Actual result contains 0 at first element, which i wrong!!!
// CHECK: 99
// CHECK: 99
// CHECK: 99
// CHECK: 99
// CHECK: 99
// CHECK: 99
// CHECK: 99
// CHECK: 99
// CHECK: 99
// CHECK: 99

void fill_array(int *a, int len, int val)
{
    int i;

    for (i = 0; i < len; i++)
    {
        a[i] = val;
    }
}

void print_array(int *a, int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        puti(a[i]);
        putc('\n');
    }
}

int main()
{
    int a[10];
    int i;

    fill_array(a, 10, 99);

    print_array(a, 10);

    return 0;
}
