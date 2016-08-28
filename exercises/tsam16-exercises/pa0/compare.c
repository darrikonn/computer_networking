#include <stdio.h>
#include <string.h>

int main() {
    FILE *f_ptr1, *f_ptr2;
    char c1[128], c2[128];
    if ((f_ptr1 = fopen("input.txt", "r")) == NULL) {
        printf("Error opening the file\n");
        return -1;
    }
    if ((f_ptr2 = fopen("input2.txt", "r")) == NULL) {
        printf("Error opening the file\n");
        return -1;
    }

    while ((fgets(c1, 128, f_ptr1), fgets(c2, 128, f_ptr2)), c1 != NULL || c2 != NULL) {
        if (strcmp(c1, c2) != 0) {
            printf("%s\n", c1);
            printf("%s\n", c2);
            break;
        }
    }

    fclose(f_ptr1);
    fclose(f_ptr2);

    return 0;
}
