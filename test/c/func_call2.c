// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 465

int sum(int x, int y)
{
    int s;
    s = x + y;
    return s;
}

int main()
{
    int a, b, c;
    b = 22;
    c = 33;
    a = (100 + sum(b, c)) * 3;
    puti(a);
    putc('\n');
    return 0;
}
