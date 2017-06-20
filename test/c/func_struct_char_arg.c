// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: a
// CHECK: b

struct S
{
    char x, y;
};

void set(struct S *s)
{
    s->x = 'a';
    s->y = 'b';
}

void print(struct S s)
{
    putc(s.x);
    putc('\n');
    putc(s.y);
    putc('\n');
}

int main()
{
    struct S s;

    set(&s);

    print(s);

    return 0;
}
