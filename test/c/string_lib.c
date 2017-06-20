void strcpy(char *s, char *t)
{
    int i = 0;

    s[i] = t[i];
    while (s[i] != '#')
    {
        s[i] = t[i];
        i++;
    }
}

void prints(char *s)
{
    int i = 0;

    while (s[i] != '#')
    {
        putc(s[i]);
        i++;
    }
}

int main()
{
    char str1[10];
    char str2[10];

    str2[0] = 'a';
    str2[1] = 'b';
    str2[2] = 'c';
    str2[3] = 'd';
    str2[4] = '#';

    strcpy(str1, str2);

    prints(str1);

    return 0;
}
