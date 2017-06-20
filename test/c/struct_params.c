// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 1022

struct CStruct
{
    int x, y, z, k;
};

int calc(struct CStruct p, int i) { return p.x + p.y + p.z + p.k + i; }

int main()
{
    struct CStruct p;
    int res;

    p.x = 100;
    p.y = 200;
    p.z = 300;
    p.k = 400;
    res = calc(p, 22);
    puti(res);
    putc('\n');

    return 0;
}
