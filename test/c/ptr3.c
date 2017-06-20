// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 22

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
    int cont;

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

    cont = 1;
    while (cont)
    {
        if (p->size >= nunits)
        { // big enough
            if (p->size == nunits)
            { // exactly
                prevp->next = p->next;
            }
            else
            {
                p->size -= nunits;
                p = (struct header_t *)((int)p + p->size);
                p->size = nunits;
            }
            freep = prevp;
            return (void *)((int)p * 8);
        }
        if (p == freep)
        {
            cont = 0;
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

int main()
{
    int **p;

    p = (void *)malloc(sizeof(int));
    *p = (int *)malloc(sizeof(int));

    **p = 22;

    puti(**p);
    putc('\n');

    return 0;
}
