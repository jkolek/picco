int sum(int a, int b);

int calculate(int x, int y)
{
    int res;
    res = sum(x, y);
    return res;
}

int main()
{
    int p, q, r;
    p = 102;
    q = 345;
    r = calculate(p, q);
    puti(r);
    putc('\n');
    return 0;
}
