int main()
{
    int res;
    int acc, n;

    n = 5;
    acc = 1;
    while (n > 0)
    {
        acc = acc * n;
        n--;
        puti(n);
        putc('\n');
    }

    res = acc;
    puti(res);
    putc('\n');
    return 0;
}
