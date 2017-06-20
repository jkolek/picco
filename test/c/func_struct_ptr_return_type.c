struct Point
{
    int x, y;
};

int malloc(int size);

struct Point *make_point(int x, int y)
{
    struct Point *res;
    res->x = x;
    res->y = y;
    res = (struct Point *)malloc(sizeof(struct Point));
    return res;
}

int main()
{
    struct Point *p;
    p = make_point(100, 200);
    return 0;
}
