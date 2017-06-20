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
    int i, j, tmp, done;

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

    for (i = 1; i < 10; i++)
    {
        if (a[i] < a[i - 1])
        {
            tmp = a[i];
            j = i;
            do
            {
                j--;
                a[j + 1] = a[j];
                if (j == 0)
                {
                    done = 1;
                }
                else
                {
                    if (a[j - 1] < tmp)
                        done = 1;
                    else
                        done = 0;
                }
            } while (!done);

            a[j] = tmp;
        }
    }

    for (i = 0; i < 10; i++)
    {
        puti(a[i]);
        putc('\n');
    }

    return 0;
}
