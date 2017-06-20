// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 300

int add(int x, int y) { return x + y; }

int main()
{
    int res;
    int (*foo)(int a, int b);

    foo = &add;

    res = (*foo)(100, 200);

    puti(res);
    putc('\n');

    return 0;
}
