struct Point1
{
    int x;
};

struct Point2
{
    struct Point1 *p1;
};

struct Point3
{
    struct Point2 p2;
};

struct Point4
{
    struct Point3 *p3;
};

int main()
{
    // struct Point4 *p4;
    // struct Point3 p3;
    // p4.p3.p2.p1.x = 22;
    // p3.p2.p1.x = 22;
    // puti(p4.p3.p2.p1.x);
    struct Point1 s1;
    struct Point2 s2;
    struct Point3 s3;
    struct Point3 *p3;
    p3 = &s3;
    p3->p2.p1 = &s1;
    p3->p2.p1->x = 33;
    puti(p3->p2.p1->x);
    putc('\n');
    return 0;
}
