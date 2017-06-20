int isnumber(char ch)
{
    if (ch >= '0' && ch <= '9')
        return 1;
    return 0;
}

int isalphanum(char ch)
{
    if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z'))
        return 1;
    return 0;
}

int isalpha(char ch)
{
    if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
        return 1;
    return 0;
}

void checkNumber(char ch)
{
    if (isnumber(ch))
        putc('t');
    else
        putc('f');

    putc('\n');
}

void checkAlphaNum(char ch)
{
    if (isalphanum(ch))
        putc('t');
    else
        putc('f');

    putc('\n');
}

void checkAlpha(char ch)
{
    if (isalpha(ch))
        putc('t');
    else
        putc('f');

    putc('\n');
}

int main()
{
    checkNumber('0'); // t
    checkNumber('5'); // t
    checkNumber('9'); // t
    checkNumber('a'); // f
    checkNumber('b'); // f
    checkNumber(';'); // f
    checkNumber('.'); // f
    putc('\n');

    checkAlphaNum('0'); // t
    checkAlphaNum('5'); // t
    checkAlphaNum('9'); // t
    checkAlphaNum('a'); // t
    checkAlphaNum('b'); // t
    checkAlphaNum(';'); // f
    checkAlphaNum('.'); // f
    putc('\n');

    checkAlpha('0'); // f
    checkAlpha('5'); // f
    checkAlpha('9'); // f
    checkAlpha('a'); // t
    checkAlpha('b'); // t
    checkAlpha(';'); // f
    checkAlpha('.'); // f
    putc('\n');

    return 0;
}
