// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 1
// CHECK: 2
// CHECK: 4

int main()
{
    puti(sizeof(char));
    putc('\n');

    puti(sizeof(short));
    putc('\n');

    puti(sizeof(int));
    putc('\n');

    return 0;
}
