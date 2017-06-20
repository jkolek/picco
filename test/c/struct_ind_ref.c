// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 100
// CHECK: 200
// CHECK: 300
// CHECK: 400
// CHECK: 500

struct s1_s
{
    int x;
    int y;
};

struct s2_s
{
    int y;
    struct s1_s s1;
};

struct s3_s
{
    int z;
    struct s2_s s2;
};

struct s4_s
{
    int i;
    struct s3_s s3;
};

int main()
{
    struct s4_s s;
    s.s3.s2.s1.x = 100;
    s.s3.s2.s1.y = 200;
    s.s3.s2.y = 300;
    s.s3.z = 400;
    s.i = 500;
    puti(s.s3.s2.s1.x);
    putc('\n');
    puti(s.s3.s2.s1.y);
    putc('\n');
    puti(s.s3.s2.y);
    putc('\n');
    puti(s.s3.z);
    putc('\n');
    puti(s.i);
    putc('\n');
    return 0;
}
