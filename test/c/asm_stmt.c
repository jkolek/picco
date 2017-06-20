int main()
{
    asm("add $9, $1, $5");
    asm("addu $0, $0, $31");
    asm("subu $10, $11, $12");
    asm("lw $10, 0($1)");
    asm("sw $10, 0($1)");

    return 0;
}
