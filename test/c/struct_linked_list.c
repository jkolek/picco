// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 2
// CHECK: 4
// CHECK: 6

struct Element;

struct Element
{
    int info;
    struct Element *next;
} e0, e1, e2, dumpElem;

int main()
{
    struct Element *e_ptr;
    struct Element *tmp;

    e_ptr = &e0;

    e0.info = 2;
    e0.next = &e1;

    e1.info = 4;
    e1.next = &e2;

    e2.info = 6;
    e2.next = &dumpElem;

    dumpElem.info = 999;

    tmp = e_ptr;
    while (tmp->info != 999)
    {
        puti(tmp->info);
        putc('\n');
        tmp = tmp->next;
    }

    return 0;
}
