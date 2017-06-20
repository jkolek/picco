// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 7

int str_size(char s[])
{
    int i = 0;
    while (s[i] != '\0')
        i++;
    return i;
}

int main()
{
    char s[10];
    s[0] = 'c';
    s[1] = 'a';
    s[2] = 't';
    s[3] = 'f';
    s[4] = 'i';
    s[5] = 's';
    s[6] = 'h';
    s[7] = '\0';
    puti(str_size(s));
    putc('\n');
    return 0;
}
