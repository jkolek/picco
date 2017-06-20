void f(int x)
{
    if (x == 0)
        return;
    putc('x');
    putc(' ');
    putc('!');
    putc('=');
    putc(' ');
    putc('0');
    putc('\n');
}

int main()
{
    f(1);
    f(0);
    return 0;
}
