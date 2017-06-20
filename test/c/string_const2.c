// Required GNU_ABI

int puts(char *str);

int main()
{
    char *str;
    char *str1;

    str = "This is a string.";
    str1 = "This is an another string.";

    puts(str);
    puts(str1);

    puts("Hello world!!!");

    return 0;
}
