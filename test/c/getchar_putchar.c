// Required GNU_ABI

int getchar();
int putchar(int ch);

int main()
{
    int n;

    while (n != 'q')
    {
        n = getchar();
        putchar(n);
    }
    putchar('\n');

    return 0;
}
