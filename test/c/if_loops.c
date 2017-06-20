// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 0
// CHECK: -1
// CHECK: -3
// CHECK: -6
// CHECK: -10
// CHECK: -15
// CHECK: -21
// CHECK: -28
// CHECK: -36
// CHECK: -45
// CHECK: -55
// CHECK: -66
// CHECK: -78
// CHECK: -91
// CHECK: -105
// CHECK: -120
// CHECK: -136
// CHECK: -153
// CHECK: -171
// CHECK: -190
// CHECK: -210
// CHECK: -231

int main()
{
    int a, b, i, n;

    a = 0;
    b = 0;

    if (a > 0)
    {
        i = 0;
        while (i < 10)
        {
            b = b + i;
            i++;
            putc(b);
            putc('\n');
        }
    }
    else if (a < 0)
    {
        i = 10;
        do
        {
            b = b * i;
            i--;
            puti(b);
            putc('\n');
        } while (i > 0);
    }
    else
    {
        if (a == b)
        {
            n = 0;
            while (n < 22)
            {
                b = b - n;
                n++;
                puti(b);
                putc('\n');
            }
        }
    }
    return b;
}
