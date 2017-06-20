// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 97

union un_type {
    char a;
    int b;
} un1;

int main()
{
    un1.a = 'a';
    puti(un1.b);
    putc('\n');
    return 0;
}
