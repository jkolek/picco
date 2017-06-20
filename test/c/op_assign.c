// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 80
// CHECK: 5
// CHECK: 3
// CHECK: 24
// CHECK: 16
// CHECK: 8
// CHECK: 2
// CHECK: 5
// CHECK: 6
// CHECK: 6

int main()
{
    int x, y;

    y = 4;

    x = 20;
    x *= y;
    puti(x);
    putc('\n');

    x = 20;
    x /= y;
    puti(x);
    putc('\n');

    x = 23;
    x %= y;
    puti(x);
    putc('\n');

    x = 20;
    x += y;
    puti(x);
    putc('\n');

    x = 20;
    x -= y;
    puti(x);
    putc('\n');

    x = 4;
    x <<= 1;
    puti(x);
    putc('\n');

    x = 4;
    x >>= 1;
    puti(x);
    putc('\n');

    x = 7;
    x &= 5;
    puti(x);
    putc('\n');

    x = 4;
    x ^= 2;
    puti(x);
    putc('\n');

    x = 4;
    x |= 2;
    puti(x);
    putc('\n');

    return 0;
}
