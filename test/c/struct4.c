// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 66
// CHECK: 2048

struct Element
{
    int info;
    int size;
} e;

int main()
{
    struct Element *e_ptr;
    int x;
    int *p;

    e_ptr = &e;

    e_ptr->info = 66;
    e_ptr->size = 2048;

    puti(e.info);
    putc('\n');

    puti(e.size);
    putc('\n');

    return 0;
}
