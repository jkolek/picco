int main()
{
    int a[4];
    int i, x;

    a[0] = 33;
    a[1] = 22;
    a[2] = 4;
    a[3] = 5;

    i = 0;
    while (i < 4)
    {
        x = a[i];
        puti(x);
        putc('\n');
        i++;
    }
}
