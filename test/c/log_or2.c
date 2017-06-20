// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 100
// CHECK: 222
// CHECK: 222

int main()
{
    int x, y, z, a, res;

    x = 0;
    y = 22;
    z = 0;

    /* The condition z == 0 is satisfied. */
    if (x == 11 || y == 0 || z == 33 || (x == 0 && z == 1) || z == 0)
        res = 100;

    puti(res); /* res == 100 */
    putc('\n');

    /* The condition (x == 0 && z == 0) is satisfied. */
    if (x == 11 || y == 0 || z == 33 || (x == 0 && z == 0) || z == 1)
        res = 222;

    puti(res); /* res == 222 */
    putc('\n');

    /* No condition should is satisfied. */
    if (x == 11 || y == 0 || z == 33 || (x == 1 && z == 1) || z == 1)
        res = 333;

    puti(res); /* res == 222 */
    putc('\n');

    return 0;
}
