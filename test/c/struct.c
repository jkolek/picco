// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 579

struct Point
{
    int x;
    int y;
} p;

int main()
{
    int sum;
    p.x = 123;
    p.y = 456;

    sum = p.y + p.x;
    puti(sum);
    putc('\n');
    return 0;
}
