// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 97
// CHECK: b

enum ObjectType
{
    TYPE_SYMBOL,
    TYPE_INTEGER,
    TYPE_PAIR
};

struct integer_t
{
    int type;
    int ival;
};

struct symbol_t
{
    int type;
    char symbol;
};

union object_t;

struct pair_t
{
    int type;
    union object_t *car, *cdr;
};

/* 'object_t' is union of all the object types. */
union object_t {
    struct integer_t integer;
    struct symbol_t symbol;
    struct pair_t pair;
} obj1, obj2, obj3;

int main()
{
    union object_t *obj1_p, *obj2_p, *obj3_p;

    obj1_p = &obj1;
    obj1_p->integer.type = TYPE_INTEGER;
    obj1_p->integer.ival = 256;

    obj2_p = &obj2;
    obj2_p->symbol.type = TYPE_SYMBOL;
    obj2_p->symbol.symbol = 'x';

    obj3_p = &obj3;
    obj3_p->pair.type = TYPE_PAIR;
    obj3_p->pair.car = obj1_p;
    obj3_p->pair.cdr = obj2_p;

    puti(sizeof(obj1_p));
    putc('\n');

    puti(obj1_p->integer.ival);
    putc('\n');

    puti(obj2_p->symbol.symbol);
    putc('\n');

    return 0;
}
