// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 0
// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 4
// CHECK: 5
// CHECK: 6
// CHECK: 7
// CHECK: 8
// CHECK: 9

int a[10];

int main()
{
    int i;
    i = 0;
    while (i < 10)
    {
        a[i] = i;
        i++;
    }
    i = 0;
    while (i < 10)
    {
        puti(a[i]);
        putc('\n');
        i++;
    }
}
