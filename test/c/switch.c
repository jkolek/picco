int main()
{
    int x, y;

    x = 2;

    switch (x)
    {
        case 1:
            y = 10;
            break;
        case 1:
            y = 20;
            break;
        default:
            y = 30;
            break;
    }

    puti(y);
    putc('\n');

    return 0;
}
