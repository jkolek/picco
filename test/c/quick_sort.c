int partition(int *arr, int lo, int hi)
{
    int x, i, j, temp;

    x = arr[hi];
    i = lo;

    /*  for (j = lo; j < hi; j++)
      {
        if (arr[j] <= x)
        {
          i++;
          temp = arr[i];
          arr[i] = arr[j];
          arr[j] = temp;
        }
      }*/

    j = lo;

    while (j < hi)
    {
        if (arr[j] <= x)
        {
            temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
            i++;
        }
        j++;
    }

    temp = arr[hi];
    arr[hi] = arr[i];
    arr[i] = temp;

    return i;
}

void quick_sort(int *arr, int lo, int hi)
{
    int p;

    if (lo < hi)
    {
        p = partition(arr, lo, hi);
        quick_sort(arr, lo, p - 1);
        quick_sort(arr, p + 1, hi);
    }
}

void fill(int *arr, int size)
{
    int i;

    for (i = 0; i < size; i++)
    {
        if (i % 3 == 0)
        {
            arr[i] = i + 100;
        }
        else if (i % 2 == 0)
        {
            arr[i] = i + 50;
        }
        else
        {
            arr[i] = i + 20;
        }
    }
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

void main()
{
    int arr[10];

    fill(arr, 10);

    print(arr, 10);

    putc('\n');

    quick_sort(arr, 0, 9);

    print(arr, 10);
}
