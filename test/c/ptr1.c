// Test a pointer arithmetic.
int a[10];

int main()
{
    int i, n;
    int *p;
    int x;

    i = 33;
    p = &a;
    // x = *p;

    a[0] = 55;
    a[1] = 66;
    a[2] = 77;
    a[3] = 88;
    a[4] = 99;
    a[5] = 100;

    n = 0;
    while (n < 10)
    {
        puti(*p);
        putc('\n');
        puti(p);
        putc('\n');
        p++;
        n++;
    }

    // puti(*p);
    // putc('\n');

    // puti(x);
    // putc('\n');

    return 0;
}
