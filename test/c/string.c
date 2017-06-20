// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: abcdefghi

int main()
{
    char str[10];
    int i;

    for (i = 0; i < 9; i++)
        str[i] = 'a' + i;

    str[9] = '\n';

    for (i = 0; i < 10; i++)
        putc(str[i]);

    return 0;
}
