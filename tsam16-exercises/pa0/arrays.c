#include <stdio.h>

int a[10];

int main()
{
    printf("%p\n", a);
    printf("%p\n", &a[0]);
    printf("%p\n", &a);
    printf("%zu\n", sizeof(a));
    printf("%zu\n", sizeof(&a));
    printf("%zu\n", sizeof(a[0]));
    printf("%zu\n", sizeof(&a[0]));
    printf("%zu\n", sizeof(*(a)));
    printf("%zu\n", sizeof(*(&a)));
    printf("%zu\n", sizeof(*(&a[0])));

    return 0;
}
