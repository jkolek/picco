// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 5
// CHECK: 10
// CHECK: 14
// CHECK: 15
// CHECK: 22
// CHECK: 30
// CHECK: 32
// CHECK: 1
// CHECK: 2
// CHECK: 3
// CHECK: 5
// CHECK: 10
// CHECK: 14
// CHECK: 15
// CHECK: 22
// CHECK: 30
// CHECK: 32

void quick_sort(int *arr)
{
    int i, j, tmp, done;

    for (i = 1; i < 10; i++)
    {
        if (arr[i] < arr[i - 1])
        {
            tmp = arr[i];
            j = i;
            do
            {
                j--;
                arr[j + 1] = arr[j];
                if (j == 0)
                    done = 1;
                else
                {
                    if (arr[j - 1] < tmp)
                        done = 1;
                    else
                        done = 0;
                }
            } while (!done);

            arr[j] = tmp;
        }
    }
}

void bubble_sort(int *arr)
{
    int i, j, tmp;

    for (i = 0; i < 9; i++)
    {
        for (j = 9; j >= i + 1; j--)
        {
            if (arr[j] < arr[j - 1])
            {
                tmp = arr[j - 1];
                arr[j - 1] = arr[j];
                arr[j] = tmp;
            }
        }
    }
}

void sort(int *arr, char type)
{
    void (*sort_fn)(int *arr);

    if (type == 'q')
        sort_fn = &quick_sort;
    else if (type == 'b')
        sort_fn = &bubble_sort;
    else
        return;

    sort_fn(arr);
}

void fill(int *arr)
{
    arr[0] = 32;
    arr[1] = 2;
    arr[2] = 1;
    arr[3] = 3;
    arr[4] = 22;
    arr[5] = 15;
    arr[6] = 14;
    arr[7] = 30;
    arr[8] = 5;
    arr[9] = 10;
}

void print(int *arr, int size)
{
    int i;

    for (i = 0; i < size; i++)
    {
        puti(arr[i]);
        putc('\n');
    }
}

int main()
{
    int a[10];
    int b[10];

    fill(a);
    fill(b);

    sort(a, 'q');
    sort(b, 'b');

    print(a, 10);
    print(b, 10);
}
