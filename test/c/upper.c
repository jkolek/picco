// Required GNU_ABI

int getchar();
int putchar(int ch);
int isalpha(int ch);

int isupper(int ch)
{
    if (ch >= 'A' && ch <= 'Z')
        return 1;
    return 0;
}

int main()
{
    int c;
    int diff = ('a' - 'A');

    while (c != 'q')
    {
        c = getchar();
        if (isalpha(c))
        {
            if (isupper(c))
                putchar(c + diff);
            else
                putchar(c - diff);
            putchar('\n');
        }
    }
    putchar('\n');

    return 0;
}
