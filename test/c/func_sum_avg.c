int sum(int x, int y) { return x + y; }

int avg(int x, int y, int z) { return (x + y + z) / 3; }

void main()
{
    int x;
    int y;
    int z;

    x = 20;
    y = 10;

    if (x == y)
    {
        z = sum(x, y);
    }
    else
    {
        z = avg(x, y, 30);
    }
    puti(z);
    putc('\n');
}
