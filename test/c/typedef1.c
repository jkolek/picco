typedef struct Token_s
{
    int kind;
} Token_t;

void main()
{
    Token_t tok;

    tok.kind = 10;
    puti(tok.kind);
    putc('\n');
}
