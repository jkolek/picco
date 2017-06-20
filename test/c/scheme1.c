// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: ( + . ( 20 . ( 10 . '() ) ) )
// CHECK:
// CHECK: ( ( c . 44 ) . ( ( b . 33 ) . ( ( a . 22 ) . '() ) ) )

/*----------------------------------------------------------------------------*/
/*  Scheme data structures                                                    */
/*----------------------------------------------------------------------------*/

enum ObjectType
{
    TYPE_SYMBOL,
    TYPE_INTEGER,
    TYPE_OPERATOR,
    TYPE_PAIR
};

struct object_t;

struct object_t
{
    int type;
    int ival;
    char symbol;
    struct object_t *car, *cdr;
};

/*----------------------------------------------------------------------------*/
/*  Heap allocation                                                           */
/*----------------------------------------------------------------------------*/

struct header_t;

struct header_t
{
    struct header_t *next;
    int size;
};

struct header_t base;
struct header_t *freep;

void init_mem() { freep->size = 1024; }

void *malloc(int nbytes);

/*void free(void *p)
{

}*/

/* -------------------------------------------------------------------------- */

char stream[256];
int stream_idx;

char getnbc();

void ungetc();

/*----------------------------------------------------------------------------*/
/*  Helper functions                                                          */
/*----------------------------------------------------------------------------*/

int isspace(char ch);
int isdigit(char ch);
int isalpha(char ch);
int isealpha(char ch);

struct object_t *objUndefined;
struct object_t *objEmpty;
struct object_t *objError;

/*----------------------------------------------------------------------------*/
/*  Make functions                                                            */
/*----------------------------------------------------------------------------*/

struct object_t *make_integer(int value);

struct object_t *make_symbol(char value);

struct object_t *make_pair(struct object_t *car, struct object_t *cdr);

/*
  (caar x) = (car (car x))
*/
struct object_t *lookup_in_frame(struct object_t *frame,
                                 struct object_t *symbol);

struct object_t *lookup(struct object_t *env, struct object_t *symbol)
{
    struct object_t *binding;
    while (env != objEmpty)
    {
        binding = lookup_in_frame(env->car, symbol);
        if (binding != objUndefined)
            return binding;
        env = env->cdr;
    }
    return objUndefined;
}

void define(struct object_t *env,
            struct object_t *symbol,
            struct object_t *value);

void print(struct object_t *obj);

/* TODO: Change 'int' to 'struct object_t *'. */
struct object_t *eval(struct object_t *env, struct object_t *obj);

void print(struct object_t *obj);

/*----------------------------------------------------------------------------*/
/*  Read functions                                                            */
/*----------------------------------------------------------------------------*/

// FIXME: This shouldn't be a type error.
// struct object_t *read();

struct object_t *read_integer(char ch);

struct object_t *read_symbol(char ch);

struct object_t *read_list();

struct object_t *read();

void init();

/* This is temporary function */
void init_stream();

/* -------------------------------------------------------------------------- */

void print_env(struct object_t *env);

int main()
{
    struct object_t *obj0, *obj1, *obj2, *obj3, *obj4, *obj5, *res;
    struct object_t *obj6;
    struct object_t *env, *s1, *s2, *s3, *v1, *v2, *v3;

    /* ( + . ( 10 . ( 20 . '() ) ) */

    obj0 = make_symbol('+');
    obj1 = make_integer(10);
    obj2 = make_integer(20);
    obj3 = make_pair(obj1, objEmpty);
    obj4 = make_pair(obj2, obj3);
    obj5 = make_pair(obj0, obj4);

    print(obj5);
    putc('\n');

    obj6 = read();
    print(obj6);
    putc('\n');

    s1 = make_symbol('a');
    s2 = make_symbol('b');
    s3 = make_symbol('c');
    v1 = make_integer(22);
    v2 = make_integer(33);
    v3 = make_integer(44);
    env = make_pair(objEmpty, objEmpty);
    define(env, s1, v1);
    define(env, s2, v2);
    define(env, s3, v3);
    print_env(env);

    res = eval(env, obj5);

    // Picco BUG
    // define(env, make_symbol('a'), make_integer(33));
    // define(env, make_symbol('b'), make_integer(44));

    return 0;
}
