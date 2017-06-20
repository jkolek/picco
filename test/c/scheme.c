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

struct object_s;

struct object_s
{
    int type;
    int ival;
    char symbol;
    struct object_s *car, *cdr;
};

typedef struct object_s Object_t;

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

void *malloc(int nbytes)
{
    struct header_t *p, *prevp;
    int nunits;

    nunits =
        (nbytes + sizeof(struct header_t) - 1) / sizeof(struct header_t) + 1;
    prevp = freep;
    if (prevp == NULL)
    {
        prevp = &base;
        freep = &base;
        freep->size = 1024;
        base.next = &base;
    }

    p = prevp->next;

    while (1)
    {
        if (p->size >= nunits)
        { /* big enough */
            if (p->size == nunits)
            { /* exactly */
                prevp->next = p->next;
            }
            else
            {
                p->size -= nunits;
                p = (struct header_t *)(p + p->size);
                p->size = nunits;
            }
            freep = prevp;
            return (void *)((int)p * 8);
        }
        if (p == freep)
        {
            break;
        }
        else
        {
            prevp = p;
            p = p->next;
        }
    }

    putc('!');
    putc('\n');

    return NULL;
}

/*void free(void *p)
{

}*/

/* -------------------------------------------------------------------------- */

char stream[256];
int stream_idx;

char getnbc()
{
    char res = stream[stream_idx];

    stream_idx++;

    return res;
}

void ungetc() { stream_idx--; }

/*----------------------------------------------------------------------------*/
/*  Helper functions                                                          */
/*----------------------------------------------------------------------------*/

int isspace(char ch)
{
    if (ch == ' ' || ch == '\n')
        return 1;

    return 0;
}

int isdigit(char ch)
{
    if (ch >= '0' && ch <= '9')
        return 1;

    return 0;
}

int isalpha(char ch)
{
    if (ch >= 'a' && ch <= 'z')
        return 1;

    return 0;
}

int isealpha(char ch)
{
    if (ch >= 'a' && ch <= 'z')
        return 1;

    return 0;
}

struct object_s *objUndefined;
struct object_s *objEmpty;
struct object_s *objError;

/*----------------------------------------------------------------------------*/
/*  Make functions                                                            */
/*----------------------------------------------------------------------------*/

struct object_s *make_integer(int value)
{
    struct object_s *obj;

    obj = (struct object_s *)malloc(sizeof(struct object_s));
    obj->type = TYPE_INTEGER;
    obj->ival = value;

    return obj;
}

struct object_s *make_symbol(char value)
{
    struct object_s *obj;

    obj = (struct object_s *)malloc(sizeof(struct object_s));
    obj->type = TYPE_SYMBOL;
    obj->symbol = value;

    return obj;
}

struct object_s *make_pair(struct object_s *car, struct object_s *cdr)
{
    struct object_s *obj;

    obj = (struct object_s *)malloc(sizeof(struct object_s));
    obj->type = TYPE_PAIR;
    obj->car = car;
    obj->cdr = cdr;

    return obj;
}

/*
  (caar x) = (car (car x))
*/
struct object_s *lookup_in_frame(struct object_s *frame,
                                 struct object_s *symbol)
{
    while (frame != objEmpty)
    {
        if (frame->car->car == symbol)
            return frame->car;
        frame = frame->cdr;
    }

    return objUndefined;
}

struct object_s *lookup(struct object_s *env, struct object_s *symbol)
{
    struct object_s *binding;

    while (env != objEmpty)
    {
        binding = lookup_in_frame(env->car, symbol);
        if (binding != objUndefined)
            return binding;
        env = env->cdr;
    }

    return objUndefined;
}

void define(struct object_s *env,
            struct object_s *symbol,
            struct object_s *value)
{
    struct object_s *binding;

    binding = lookup_in_frame(env->car, symbol);
    if (binding != objUndefined)
        binding->cdr = value;
    else
        env->car = make_pair(make_pair(symbol, value), env->car);
}

void print(struct object_s *obj);

/* TODO: Change 'int' to 'struct object_s *'. */
struct object_s *eval(struct object_s *env, struct object_s *obj)
{
    struct object_s *res, *ptr, *car, *cdr, *tmp;

    if (obj->type == TYPE_INTEGER)
    {
        res = obj;
    }
    else if (obj->type == TYPE_SYMBOL)
    {
        tmp = lookup(env, obj->symbol);
        if (tmp != objUndefined)
            return tmp->car;
    }
    else if (obj->type == TYPE_OPERATOR)
    {
        ptr = obj->cdr;
        while (ptr != objEmpty)
        {
            //
            ptr = ptr->cdr;
        }
    }
    else if (obj->type == TYPE_PAIR)
    {
        car = eval(env, obj->car);
        cdr = eval(env, obj->cdr);
    }

    return res;
}

void print(struct object_s *obj)
{
    if (obj->type == TYPE_INTEGER)
    {
        puti(obj->ival);
    }
    else if (obj->type == TYPE_SYMBOL)
    {
        putc(obj->symbol);
    }
    else if (obj->type == TYPE_PAIR)
    {
        putc('(');
        putc(' ');
        print(obj->car);
        putc(' ');
        putc('.');
        putc(' ');
        if (obj->cdr != objEmpty)
        {
            print(obj->cdr);
        }
        else
        {
            putc('\'');
            putc('(');
            putc(')');
        }
        putc(' ');
        putc(')');
    }
}

/*----------------------------------------------------------------------------*/
/*  Read functions                                                            */
/*----------------------------------------------------------------------------*/

// FIXME: This shouldn't be a type error.
// struct object_s *read();

Object_t *read_integer(char ch)
{
    int value = 0;

    do
    {
        value = value * 10 + ch - '0';
        ch = getnbc();
    } while (isdigit(ch));

    ungetc();

    return make_integer(value);
}

Object_t *read_symbol(char ch) { return make_symbol(ch); }

Object_t *read_list()
{
    // FIXME: object wasn't detected as non-existing type!!!
    // struct object *list, *new, *end;
    struct object_s *list, *new, *end;
    char ch;

    list = objEmpty;
    end = NULL;
    while (1)
    {
        ch = getnbc();
        if (ch == ')' || ch == '.' || ch == '#')
        { // TODO: Replace '#' with the EOF
            goto break_label;
        }
        ungetc();
        new = make_pair(read(), objEmpty);
        if (list == objEmpty)
        {
            list = new;
            end = new;
        }
        else
        {
            end->cdr = new;
            end = new;
        }
    }
break_label:
    if (ch == '.')
    {
        if (list == objEmpty)
        {
            putc('!');
            putc('\n');
        }
        end->cdr = read();
        ch = getnbc();
    }

    if (ch != ')')
    {
        putc(')');
        putc('?');
        putc('\n');
    }

    return list;
}

Object_t *read()
{
    char ch = getnbc();

    if (isdigit(ch))
    {
        return read_integer(ch);
    }
    else if (ch == '(')
    {
        return read_list();
    }

    if (isalpha(ch) || isealpha(ch))
        return read_symbol(ch);

    return objError;
}

void init()
{
    objEmpty = (struct object_s *)malloc(sizeof(struct object_s));
    objError = (struct object_s *)malloc(sizeof(struct object_s));
}

/* This is temporary function */
void init_stream()
{
    stream[0] = '(';
    stream[1] = '+';
    stream[2] = ' ';
    stream[3] = '2';
    stream[4] = ' ';
    stream[5] = '7';
    stream[6] = ')';
    stream[7] = '#';
}

/* -------------------------------------------------------------------------- */

void print_env(struct object_s *env)
{
    struct object_s *tmp = env;
    while (tmp != objEmpty)
    {
        print(tmp->car);
        tmp = tmp->cdr;
    }
    putc('\n');
}

int main()
{
    Object_t *obj0, *obj1, *obj2, *obj3, *obj4, *obj5, *res;
    struct object_s *obj6;
    struct object_s *env, *s1, *s2, *s3, *v1, *v2, *v3;

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
