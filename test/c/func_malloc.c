// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: h=2720
// CHECK: p=2720
// CHECK: q=2732
// CHECK: r=2744

/*---------------------------------------------------------------------------*/
/*  Memory space allocation - "best fit" approach                            */
/*---------------------------------------------------------------------------*/

struct Point
{
    char name;
    int x, y;
};

/*  Memory structure

  +-------------+------------------------+---------------+----------------+
  | Static data | Heap meta data (1200b) | Heap data --> | <-- Stack data |
  +-------------+------------------------+---------------+----------------+

  sizeof(Heap meta data) = sizeof(free_t) x 1000 = 1200                      */

struct free_t;

struct free_t
{
    int adr, size;
    struct free_t *next;
} hdr;

/* Header pointer must be a last declared, to be last in the
   static data memory. */
struct free_t *header;

/* Initializes the header */
void init_header(int i)
{
    header = &hdr;

    /* Heap data max size */
    header->size = 4096;

    /* Heap data start address  */
    header->adr = ((int)&header) + sizeof(free_t) + 1200;

    header->next = NULL;

    putc('h');
    putc('=');
    puti(header->adr);
    putc('\n');
}

int malloc(int allocsize)
{
    int adr, best, found;
    struct free_t *tmp, *bestTmp, *trash, *prev, *bestPrev;

    /* Allocation algorithm */

    best = 4096 + 1; /* HEAP_SIZE+1 */
    found = 0;
    tmp = header;

    while (tmp != NULL)
    {
        if (tmp->size >= allocsize && tmp->size < best)
        {
            best = tmp->size;
            bestTmp = tmp;
            bestPrev = prev;
            found++;
        }
        prev = tmp;
        tmp = tmp->next;
    }

    if (found == 0)
    {
        /* Allocation error */
        putc('!');
        putc('\n');
    }
    else
    {
        tmp = bestTmp;
        prev = bestPrev;
        adr = tmp->adr;
        if (tmp->size == allocsize)
        {
            if (prev == NULL)
            {
                trash = header;
                header = header->next;
                /*free(trash); */
            }
            else
            {
                trash = tmp;
                prev->next = tmp->next;
                /*free(tmp); */
            }
        }
        else
        {
            tmp->adr += allocsize;
            tmp->size -= allocsize;
        }
    }

    return adr;
}

/* Deallocate memory space */
void free(int adr, int size)
{
    struct free_t *tmp, *prev, *p;

    tmp = header;
    prev = NULL;

    while (tmp != NULL && tmp->adr < adr)
    {
        /* Segmentation fault */
        if (tmp->adr == adr)
        {
            putc('!');
            putc('\n');
        }
        prev = tmp;
        tmp = tmp->next;
    }

    if (prev != NULL && prev->adr + prev->size == adr)
    {
        prev->size += size;
        if (tmp != NULL && adr + size == tmp->adr)
        {
            prev->size += tmp->size;
            prev->next = tmp->next;
            /*free(tmp); */
        }
    }
    else if (tmp != NULL && adr + size == tmp->adr)
    {
        tmp->adr -= size;
        tmp->size += size;
    }
    else
    {
        /*freeT p = (freeT) malloc(sizeof(struct free_t));*/
        p->adr = adr;
        p->size = size;
        if (prev == NULL)
        {
            p->next = header;
            header = p;
        }
        else
        {
            p->next = prev->next;
            prev->next = p;
        }
    }
}

int main()
{
    int allocsize;
    struct Point *p, *q, *r;

    init_header(0);

    /* Allocate an addresses */

    allocsize = sizeof(Point);

    /*p = malloc(sizeof(Point));
    q = malloc(sizeof(Point));
    r = malloc(sizeof(Point));*/

    p = (struct Point *)malloc(allocsize);
    q = (struct Point *)malloc(allocsize);
    r = (struct Point *)malloc(allocsize);

    putc('p');
    putc('=');
    puti(p);
    putc('\n');

    putc('q');
    putc('=');
    puti(q);
    putc('\n');

    putc('r');
    putc('=');
    puti(r);
    putc('\n');

    return 0;
}
