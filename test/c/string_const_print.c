void print_string(char *str)
{
    char *s = str;
    while (*s != '\0')
    {
        putc(*s);
        s++;
    }
}

void main() { print_string("Hello, world."); }
