// RUN: picco %s -o output.o
// RUN: plinker output.o
// RUN: mipselemu output.out
// CHECK: h=2824
// CHECK: p=2824
// CHECK: q=2836
// CHECK: r=2848
// CHECK: p->x=25
// CHECK: p->y=27
// CHECK: q->x=35
// CHECK: q->y=39
// CHECK: r->x=48
// CHECK: r->y=49

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

int main()
{
    int adr, best, found;
    struct free_t *tmp, *bestTmp, *trash, *prev, *bestPrev;
    int allocsize, i;

    struct Point *p, *q, *r;

    /* Initialize the header -------------------------------------------------
     */

    header = &hdr;
    header->size = 4096; /* Heap data max size       */
    header->adr = (int)header + sizeof(struct free_t) +
                  1200; /* Heap data start address  */
    header->next = NULL;

    putc('h');
    putc('=');
    puti(header->adr);
    putc('\n');

    /* -----------------------------------------------------------------------
     */

    /* Allocation algorithm --------------------------------------------------
     */

    for (i = 0; i < 3; i++)
    {
        best = 4096 + 1; /* HEAP_SIZE+1 */
        allocsize = sizeof(struct Point);
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
            // allocation error
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
                    // free(trash);
                }
                else
                {
                    trash = tmp;
                    prev->next = tmp->next;
                    // free(tmp);
                }
            }
            else
            {
                tmp->adr += allocsize;
                tmp->size -= allocsize;
            }
        }

        /* Set the allocated address */
        if (i == 0)
            p = (struct Point *)adr;
        else if (i == 1)
            q = (struct Point *)adr;
        else
            r = (struct Point *)adr;
    }
    /* -----------------------------------------------------------------------
     */

    /* Verify the allocated addresses ----------------------------------------
     */

    p->x = 25;
    p->y = 27;
    q->x = 35;
    q->y = 39;
    r->x = 48;
    r->y = 49;

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

    putc('p');
    putc('-');
    putc('>');
    putc('x');
    putc('=');
    puti(p->x);
    putc('\n');

    putc('p');
    putc('-');
    putc('>');
    putc('y');
    putc('=');
    puti(p->y);
    putc('\n');

    putc('q');
    putc('-');
    putc('>');
    putc('x');
    putc('=');
    puti(q->x);
    putc('\n');

    putc('q');
    putc('-');
    putc('>');
    putc('y');
    putc('=');
    puti(q->y);
    putc('\n');

    putc('r');
    putc('-');
    putc('>');
    putc('x');
    putc('=');
    puti(r->x);
    putc('\n');

    putc('r');
    putc('-');
    putc('>');
    putc('y');
    putc('=');
    puti(r->y);
    putc('\n');
    /* -----------------------------------------------------------------------
     */

    return 0;
}
