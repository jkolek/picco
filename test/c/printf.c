// TODO: void printf(const char *fmt, ...);
void printf(char *fmt)
{
    int i;

    i = 0;
    while (fmt[i] != '\0')
    {
        putc(fmt[i]);
        i++;
    }
    i = 0;
    while (fmt[i] != '\0')
    {
        if (fmt[i] == '%')
        {
            i++;
            putc(fmt[i]);
            putc('\n');
        }
        i++;
    }
}

int main()
{
    char *s;
    char *s2;
    int i;

    s = "Hello world! This is test format of %s. Test int: %d.";
    // s2 = "This is test";
    printf(s);

    /*i = 0;
    while (s[i] != '\0') {
      putc(s[i]);
      i++;
    }*/

    /*
    FIXME: This is not working.
    while (*s != '\0') {
      putc(*s);
      s++;
    }*/

    putc('\n');

    return 0;
}
