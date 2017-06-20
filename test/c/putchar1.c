// Required GNU_ABI

int putchar(int ch);

int main()
{
    int x = 65;
    while (x < 75)
    {
        putchar(x);
        x++;
    }
    putchar('\n');

    return 0;
}
