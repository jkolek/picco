void print_arr(int a[], int n)
{
    int i;

    for (i = 0; i < n; i++)
    {
        puti(a[i]);
        putc('\n');
    }
}

int main()
{
    int a[10];

    a[0] = 11;
    a[1] = 12;
    a[2] = 13;
    a[3] = 14;
    a[4] = 15;
    a[5] = 16;
    a[6] = 17;
    a[7] = 18;
    a[8] = 19;
    a[9] = 20;

    print_arr(a, 10);

    return 0;
}
