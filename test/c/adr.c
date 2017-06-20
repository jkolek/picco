struct header_t
{
    int size, info;
};

struct header_t base;

int main()
{
    struct header_t *prevp;

    prevp = &base;

    return 0;
}
