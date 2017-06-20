// Multiple function calls test

int sum2(int x, int y)
{
    int res;
    res = x + y;
    return res;
}

int sum(int a, int b, int c, int d) { return sum2(a, b) + sum2(c, d); }

int main()
{
    int p, q, r, s, res;
    p = 102;
    q = 345;
    r = 123;
    s = 54;
    res = sum(p, q, r, s);
    puti(res);
    putc('\n');
    return 0;
}
