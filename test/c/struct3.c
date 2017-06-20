// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 100
// CHECK: 33
// CHECK: 1024
// CHECK:
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
    p = &x;

    x = 100;

    e.info = 33;
    e.size = 1024;

    puti(*p);
    putc('\n');

    puti(e_ptr->info);
    putc('\n');

    puti(e_ptr->size);
    putc('\n');

    putc('\n');

    //*p = 200;
    e_ptr->info = 66;
    e_ptr->size = 2048;

    puti(e.info);
    putc('\n');

    puti(e.size);
    putc('\n');

    return 0;
}
