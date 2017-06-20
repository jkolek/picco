// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 55
// CHECK: 77

struct Point
{
    int x, y;
};

void set(struct Point *p)
{
    p->x = 55;
    p->y = 77;
}

void print(struct Point p)
{
    puti(p.x);
    putc('\n');
    puti(p.y);
    putc('\n');
}

int main()
{
    struct Point p;

    set(&p);

    print(p);

    return 0;
}
