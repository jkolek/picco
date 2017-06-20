// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 18

struct Point
{
    int x;
    int y;
} p, q;

int main()
{
    int area;
    p.x = 9;
    p.y = 5;

    q.x = 3;
    q.y = 2;

    area = (p.x - q.x) * (p.y - q.y);

    puti(area);
    putc('\n');
    return 0;
}
