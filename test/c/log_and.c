// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 100
// CHECK: 333
// CHECK: 444

int main()
{
    int x, y, z, a, res;

    x = 11;
    y = 22;
    z = 33;

    if (x == 11 && y == 22 && z == 33)
        res = 100;

    puti(res); /* res == 100 */
    putc('\n');

    a = 0;
    if (x && y && z && a)
        res = 222;
    else
        res = 333;

    puti(res); /* res == 333 */
    putc('\n');

    a = 44;
    if (x && y && z && a)
        res = 444;
    else
        res = 555;

    puti(res); /* res == 444 */
    putc('\n');

    return 0;
}
