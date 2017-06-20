// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 0
// CHECK: 1
// CHECK: 4
// CHECK: 9

struct Point;

struct Point
{
    int x, y;
    int info;
    struct Point *next;
} p0, p1, p2, p3;

int main()
{
    struct Point *ptr, *tmp;
    int n = 0;

    while (n < 4)
    {
        if (n == 0)
        {
            ptr = &p0;
            tmp = ptr;
            tmp->info = n * n;
        }
        else if (n == 1)
        {
            tmp->next = &p1;
            tmp = tmp->next;
            tmp->info = n * n;
        }
        else if (n == 2)
        {
            tmp->next = &p2;
            tmp = tmp->next;
            tmp->info = n * n;
        }
        else
        {
            tmp->next = &p3;
            tmp = tmp->next;
            tmp->info = n * n;
        }
        n++;
    }
    tmp->next = NULL;

    tmp = ptr;
    while (tmp != NULL)
    {
        puti(tmp->info);
        putc('\n');
        tmp = tmp->next;
    }

    return 0;
}
