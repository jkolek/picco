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

/*
 *  Bubble sort
 *
 *  Implementation using FOR statement.
 */

int main()
{
    int a[10];
    int i, j, tmp;

    a[0] = 32;
    a[1] = 2;
    a[2] = 1;
    a[3] = 3;
    a[4] = 22;
    a[5] = 15;
    a[6] = 14;
    a[7] = 30;
    a[8] = 5;
    a[9] = 10;

    for (i = 0; i < 9; i++)
    {
        for (j = 9; j >= i + 1; j--)
        {
            if (a[j] < a[j - 1])
            {
                tmp = a[j - 1];
                a[j - 1] = a[j];
                a[j] = tmp;
            }
        }
    }

    for (i = 0; i < 10; i++)
    {
        puti(a[i]);
        putc('\n');
    }

    return 0;
}
