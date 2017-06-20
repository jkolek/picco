void print_string(char *s)
{
    int i = 0;
    while (s[i] != '\0')
    {
        putc(s[i]);
        i++;
    }
}

int main()
{
    int n;
    char *str;
    char *str1;
    str = "This is a string.";
    str1 = "This is an another string.";
    n = 0;
    while (str[n] != '\0')
    {
        putc(str[n]);
        n++;
    }
    putc('\n');
    print_string(str1);
    putc('\n');
    // putc(*str1);
    return 0;
}
