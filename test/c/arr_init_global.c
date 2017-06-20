// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 101
// CHECK: 102
// CHECK: 103
// CHECK: 104
// CHECK: 105

int arr[5] = {101, 102, 103, 104, 105};

void main()
{
    int i = 0;

    for (i = 0; i < 5; i++)
    {
        puti(arr[i]);
        putc('\n');
    }
}
