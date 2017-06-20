// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 11
// CHECK: 21
// CHECK: 12
// CHECK: 22

void print_int(int x)
{
    puti(x);
    putc('\n');
}

void A()
{
    static int a = 10;
    a++;
    print_int(a);
}

void B()
{
    static int b = 20;
    b++;
    print_int(b);
}

int main()
{
    A();
    B();
    A();
    B();
}
