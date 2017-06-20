// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: x=22
// CHECK: y=33

struct Point
{
    int x, y;
};

void print_point(struct Point *p)
{
    putc('x');
    putc('=');
    puti(p->x);
    putc('\n');
    putc('y');
    putc('=');
    puti(p->y);
    putc('\n');
}

int main()
{
    struct Point point;

    point.x = 22;
    point.y = 33;

    print_point(&point);

    return 0;
}
