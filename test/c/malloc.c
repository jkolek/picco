// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: 0
// CHECK: 0
// CHECK: 2
// CHECK: 4
// CHECK: 6
// CHECK: 8
// CHECK: 10
// CHECK: 12
// CHECK: 14
// CHECK: 16
// CHECK: 18

struct element_t;

struct element_t
{
    int info;
    struct element_t *next;
};

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
                p = (struct header_t *)((int)p + p->size);
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
  //
}*/

void main()
{
    struct element_t *list, *tmp;
    int i;

    // init_mem();
    list = (struct element_t *)malloc(sizeof(struct element_t));

    /* Allocate elements of the list using malloc. */
    tmp = list;
    for (i = 0; i <= 10; i++)
    {
        if (i < 10)
        {
            tmp->next = (struct element_t *)malloc(sizeof(struct element_t));
            tmp = tmp->next;
            tmp->info = i + i;
        }
        else
        {
            tmp->next = NULL;
        }
    }

    tmp = list;
    while (tmp != NULL)
    {
        puti(tmp->info);
        putc('\n');
        tmp = tmp->next;
    }
}
