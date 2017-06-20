int putchar(int ch);

void print(char *s)
{
    int i = 0;

    while (s[i] != 'd')
    {
        // putc(s[i]);
        putchar(s[i]);
        i++;
    }
    // putc('\n');
    putchar('\n');
}

int main()
{
    char *s1 = "hello world";
    char s2[100];

    // print(s2);
    print(s1);

    return 0;
}
