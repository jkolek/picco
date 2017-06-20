// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: a

int main()
{
    char a;

    a = 'a';
    putc(a);
    putc('\n');

    return 0;
}
