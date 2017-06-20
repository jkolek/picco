// Required GNU_ABI

int putchar(int ch);

int main()
{
    int x = 0;
    while (x < 128)
    {
        putchar(x);
        x++;
    }
    putchar('\n');

    return 0;
}
