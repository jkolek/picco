// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 5
// CHECK: 10
// CHECK: 14
// CHECK: 15
// CHECK: 22
// CHECK: 30
// CHECK: 32

int main()
{
    int a[10];
    int i, j, tmp;

    a[0] = 32;
    a[1] = 1;
    a[2] = 3;
    a[3] = 2;
    a[4] = 22;
    a[5] = 15;
    a[6] = 10;
    a[7] = 14;
    a[8] = 30;
    a[9] = 5;

    i = 1;
    while (i < 10)
    {
        tmp = a[i];
        j = i - 1;
        while (j >= 0 && tmp < a[j])
        {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = tmp;
        i++;
    }

    i = 0;
    while (i < 10)
    {
        puti(a[i]);
        putc('\n');
        i++;
    }

    return 0;
}
