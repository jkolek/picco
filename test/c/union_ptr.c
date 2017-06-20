// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 97
// CHECK: b

union un_type {
    char a;
    int b;
} un1, un2;

int main()
{
    union un_type *un2_p;
    un1.a = 'a';
    puti(un1.b);
    putc('\n');

    un2_p = &un2;
    un2_p->b = 98;
    putc(un2_p->a);
    putc('\n');

    return 0;
}
