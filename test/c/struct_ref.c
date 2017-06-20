// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 18
// CHECK: 18
// CHECK: 6
// CHECK: 3

struct Point
{
    int x;
    int y;
} p, q;

int main()
{
    int area, area_from_res;
    struct Point res;
    struct Point *point_ref;

    point_ref = &res;

    p.x = 9;
    p.y = 5;

    q.x = 3;
    q.y = 2;

    point_ref->x = p.x - q.x;
    point_ref->y = p.y - q.y;

    area = (p.x - q.x) * (p.y - q.y);
    area_from_res = point_ref->x * point_ref->y;

    puti(area);
    putc('\n');

    puti(area_from_res);
    putc('\n');

    puti(res.x);
    putc('\n');

    puti(res.y);
    putc('\n');

    return 0;
}
