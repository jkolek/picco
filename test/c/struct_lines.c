// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 10
// CHECK: 20
// CHECK: 30
// CHECK: 40

struct Point
{
    int x, y;
} xxx;

struct Line
{
    struct Point p, q;
} line;

void print_n(int x)
{
    puti(x);
    putc('\n');
}

int main()
{
    int len;

    line.p.x = 10;
    line.p.y = 20;
    line.q.x = 30;
    line.q.y = 40;

    print_n(line.p.x);
    print_n(line.p.y);
    print_n(line.q.x);
    print_n(line.q.y);

    return 0;
}
