// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: b

void main()
{
    char str[10];
    char *p;
    char ch;

    str[0] = 'a';
    // str[1] = 'b';
    // str[2] = 'c';
    // str[3] = '\n';

    // FIXME: This is wrong way of assigning array to pointer
    p = &str;

    // p = str;
    // p++;

    // ch = *p;
    // putc(ch);
    // putc('\n');
}
