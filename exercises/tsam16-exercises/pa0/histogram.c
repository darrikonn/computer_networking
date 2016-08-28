#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

typedef struct node {
    int data;
    int value;
    struct node* left;
    struct node* right;
} treeNode;

treeNode* insert(treeNode* node, int data) {
    if (node == NULL) {
        treeNode* temp = (treeNode*)malloc(sizeof(treeNode));
        temp->data = data;
        temp->value = 1;
        temp->left = NULL;
        temp->right = NULL;
        return temp;
    } else {
        if (node->data < data) {
            node->right = insert(node->right, data);
        } else if(node->data > data) {
            node->left = insert(node->left, data);
        }

        if (node->data == data) ++node->value;
        return node;
    }
}

treeNode* delete(treeNode* node) {
    if (node == NULL) {
        printf("no element found");
    } else {
        if (node->left != NULL) {
            node->left = delete(node->left);
        }
        if (node->right != NULL) {
            node->right = delete(node->right);
        }

        free(node);
    }
    return node;
}

void printVertical(treeNode* node) {
    if (node == NULL) {
        return;
    }

    printVertical(node->left);
    printf("%d", node->data);
    printf("%*s", 4-snprintf(0, 0, "%+d", node->data), " ");
    for (int i = 0; i < node->value; i++) {
        printf("#");
    }
    printf("\n");
    printVertical(node->right);
}

int main() {
    FILE *f_ptr;
    char s[256];
    size_t maxValue = 0;
    treeNode* root = NULL;

    if ((f_ptr = fopen("input.txt", "r")) == NULL) {
       printf("Error opening the file\n"); 
       return -1;
    }

    while((fscanf(f_ptr, "%s", s)) != EOF) {
        size_t size = strlen(s);
        root = insert(root, size);

        if (size > maxValue) {
            maxValue = size;
        }
    }
    printVertical(root);

    root = delete(root);
    fclose(f_ptr);

    return 0;
}
