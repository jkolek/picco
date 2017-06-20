// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 200
// CHECK: 33

struct Record1
{
    int info;
    int size;
} r1;

struct Record2
{
    struct Record1 rec1;
    int r2_info;
    int r2_size;
} rec2;

int main()
{
    int n;

    rec2.r2_info = 33;
    rec2.rec1.size = 200;
    rec2.rec1.info = 100;
    n = rec2.rec1.size;
    puti(n);
    putc('\n');
    puti(rec2.r2_info);
    putc('\n');

    return 0;
}
