// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 20

void test(int x)
{
    if (x < 10)
        return;
    puti(x);
    putc('\n');
}

int main()
{
    test(5);
    test(20);
    return 0;
}
