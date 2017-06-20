/*
  This test case is parsed correctly, but generated machine code is
  not correct. Break statements are not implemented, and there are
  possibly some other bugs. Anyway until bugs are fixed it's good
  test for the parser.
*/

/*typedef struct {
  int is_available;
  int size;
} MCB, *MCB_P;*/

struct memory_block_t
{
    int is_available;
    int size;
};

struct point_t
{
    char id;
    int x, y;
};

char *mem_start_p;
int max_mem;
int allocated_mem; /* This is the memory in use. */
int mcb_count;

char *heap_end;

/* So far only named enums are supported. */
enum flags1
{
    NEW_MCB,
    NO_MCB,
    REUSE_MCB
};
enum flags2
{
    FREE,
    IN_USE
};

void init_mem(char *ptr, int size_in_bytes)
{
    /* Store the ptr and size_in_bytes in global variable. */

    max_mem = size_in_bytes;
    mem_start_p = ptr;
    mcb_count = 0;
    allocated_mem = 0;
    heap_end = (char *)((int)mem_start_p + size_in_bytes);
}

/* If size of the available chunk is equal to greater than required size,
   use that chunk. */
int malloc(int elem_size)
{
    /* Check whether any chunk (allocated before) is free first */

    struct memory_block_t *p_mcb;
    int sz;
    int flag;

    p_mcb = (struct memory_block_t *)mem_start_p;
    flag = NO_MCB;

    sz = sizeof(struct memory_block_t);

    if ((elem_size + sz) > (max_mem - (allocated_mem + mcb_count * sz)))
    {
        putc('!');
        putc('\n');
        return NULL;
    }

    while ((int)heap_end > ((int)p_mcb + elem_size + sz))
    {
        if (p_mcb->is_available == 0)
        {
            if (p_mcb->size == 0)
            {
                flag = NEW_MCB;
                break;
            }
            else if (p_mcb->size > (elem_size + sz))
            {
                flag = REUSE_MCB;
                break;
            }
        }
        p_mcb = (struct memory_block_t *)((int)p_mcb + p_mcb->size);
    }

    if (flag != NO_MCB)
    {
        p_mcb->is_available = 1;

        if (flag == NEW_MCB)
        {
            p_mcb->size = elem_size + sizeof(struct memory_block_t);
            mcb_count++;
        }
        allocated_mem += elem_size;
        return (int)p_mcb + sz;
    }

    /* Returning as we could not allocate any MCB. */
    return NULL;
}

int MemEfficiency()
{
    /* Keep track of number of MCBs in a global variable. */
    return mcb_count;
}

void free(void *p)
{
    /* Mark in MCB that this chunk is free */
    struct memory_block_t *ptr;

    ptr = (struct memory_block_t *)p;
    ptr--;

    mcb_count--;
    ptr->is_available = FREE;
    allocated_mem -= (ptr->size - sizeof(struct memory_block_t));
}

void main()
{
    char buf[1024];
    char *str, *str1;

    /*memset(buf, 0, 1024);*/

    init_mem(buf, 1024);

    str = (char *)malloc(100);

    free(str);

    str1 = (char *)malloc(200);
}
