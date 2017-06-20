int main()
{
    int i = 0x01020304;
    char *ptr = (char *)&i;
    puti(*ptr);
    putc('\n');

    return 0;
}
