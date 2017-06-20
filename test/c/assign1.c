// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK:

int main()
{
    int x;

    x = 22;

    return 0;
}
