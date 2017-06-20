// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK:

int main()
{
    int x, y;

    x = (y < 22) && (y != 10);

    return 0;
}
