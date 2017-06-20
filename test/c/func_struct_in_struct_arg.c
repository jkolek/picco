struct Point
{
    int x, y;
    struct Data
    {
        unsigned info;
        char a, b;
    } d;
};

void set(struct Point *p)
{
    p->x = 55;
    p->y = 77;
    p->d.info = 1000;
    p->d.a = 'a';
    p->d.b = 'b';
}

void print(struct Point p)
{
    puti(p.x);
    putc('\n');
    puti(p.y);
    putc('\n');
    puti(p.d.info);
    putc('\n');
    puti(p.d.a);
    putc('\n');
    puti(p.d.b);
    putc('\n');
}

int main()
{
    struct Point p;

    set(&p);

    print(p);

    return 0;
}
