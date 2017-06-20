struct header_t;

struct header_t
{
    struct header_t *next;
    int size;
};

struct header_t base;
struct header_t *freep;

void init_mem() { freep->size = 1024; }

int main()
{
    init_mem();
    puti(freep->size);
    putc('\n');
    return 0;
}
