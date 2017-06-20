// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 33.333

int main()
{
    int ceo, i, x, y, m, dec, ost10;

    x = 100;
    y = 3;
    m = 3;

    ceo = x / y;
    puti(ceo);
    if (m > 0)
    {
        putc('.');
        ost10 = (x % y) * 10;
        i = 1;
        while (i <= m)
        {
            dec = ost10 / y;
            ost10 = (ost10 % y) * 10;
            puti(dec);
            i++;
        }
    }
    putc('\n');
    return 0;
}
