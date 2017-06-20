// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: +
// CHECK: +
// CHECK: 300
// CHECK: +
// CHECK: 133
// CHECK: -
// CHECK: 200

void my_int_func(int x)
{
    puti(x);
    putc('\n');
}

int add(int x, int y)
{
    putc('+');
    putc('\n');
    return x + y;
}

int sub(int x, int y)
{
    putc('-');
    putc('\n');
    return x - y;
}

int main()
{
    int res;
    int (*foo)(int a, int b);
    void (*print)(int a);

    foo = &add;
    foo(22, 33);

    res = (*foo)(100, 200);

    print = &my_int_func;
    print(res);

    res = foo(100, 33);
    (*print)(res);

    foo = &sub;
    res = (*foo)(400, 200);
    (*print)(res);

    return 0;
}
