int main()
{
    int a[5];
    int i;

    for (i = 0; i < 5; i++)
    {
        a[i] = i;
    }

    i = 0;
    while (a[i] != 4)
    {
        puti(a[i]);
        putc('\n');
        i++;
    }

    return 0;
}
