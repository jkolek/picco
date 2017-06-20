// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 18
// CHECK: 18

struct Point
{
    int x;
    int y;
} p, q;

int main()
{
    int area, area_from_res;
    struct Point res;

    p.x = 9;
    p.y = 5;

    q.x = 3;
    q.y = 2;

    res.x = p.x - q.x;
    res.y = p.y - q.y;

    area = (p.x - q.x) * (p.y - q.y);

    area_from_res = res.x * res.y;

    puti(area);
    putc('\n');

    puti(area_from_res);
    putc('\n');

    return 0;
}
