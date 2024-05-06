// Binary Tree
// Author: Thiago Prates <tsprates@hotmail.com>

#include <stdio.h>
#include <stdlib.h>

typedef struct _node {
    int data;
    struct _node* left;
    struct _node* right;
} node;

node* new_node(int data) {
    node *n = malloc(sizeof (node));
    n->data = data;
    n->left = n->right = NULL;
    return n;
}

void init_root(node **root) {
    *root = NULL;
}

void insert_node(node **n, int data) {
    if (*n == NULL) {
        *n = new_node(data);
    } else {
        if ((*n)->data > data) {
            insert_node(&((*n)->left), data);
        } else {
            insert_node(&((*n)->right), data);
        }
    }
}

int remove_node(node **n, int data) {
    node *temp, *iter_node, *parent_iter_node;

    if (*n == NULL) {
        return 0;
    } else {
        if ((*n)->data > data) {
            return remove_node(&((*n)->left), data);
        } else if ((*n)->data < data) {
            return remove_node(&((*n)->right), data);
        } else {
            if ((*n)->left == NULL && (*n)->right == NULL) {
                free(*n);
                *n = NULL;
            } else if ((*n)->left != NULL && (*n)->right == NULL) {
                temp = *n;
                *n = (*n)->left;

                free(temp);
                temp = NULL;
            } else if ((*n)->left == NULL && (*n)->right != NULL) {
                temp = *n;
                *n = (*n)->right;

                free(temp);
                temp = NULL;
            } else {
                temp = *n;

                parent_iter_node = *n;
                iter_node = (*n)->right;

                while (iter_node->left != NULL) {
                    parent_iter_node = iter_node;
                    iter_node = iter_node->left;
                }

                iter_node->left = (*n)->left;

                if (*n != parent_iter_node) {
                    iter_node->right = (*n)->right;
                    parent_iter_node->left = NULL;
                } else {
                    iter_node->right = NULL;
                }

                *n = iter_node;

                free(temp);
                temp = NULL;
            }

            return 1;
        }
    }
}

void print_inorder(node *n) {
    if (n != NULL) {
        print_inorder(n->left);
        printf("%d\n", n->data);
        print_inorder(n->right);
    }
}

void print_preorder(node *n) {
    if (n != NULL) {
        printf("%d\n", n->data);
        print_inorder(n->left);
        print_inorder(n->right);
    }
}

void print_posorder(node *n) {
    if (n != NULL) {
        print_inorder(n->left);
        print_inorder(n->right);
        printf("%d\n", n->data);
    }
}

void print_inorder_tree(node *n, int space) {
    int i = 0;
    if (n != NULL) {
        print_inorder_tree(n->left, space + 1);
        while (i++ < space) {
            printf("\t");
        }
        printf("%d\n", n->data);
        print_inorder_tree(n->right, space + 1);
    }
}

int count_nodes(node *n) {
    if (n == NULL)
        return 0;
    return 1 + count_nodes(n->left) + count_nodes(n->right);
}

int main(int argc, char** argv) {
    node *root;

    int op, value;

    init_root(&root);

    while (1) {
        printf("Binary Tree\n\n"
                "Choose an option:\n"
                "[1] New node\n"
                "[2] Delete node\n"
                "[3] Show nodes\n"
                "[0] Quit\n");

        scanf("%d", &op);

        switch (op) {
            case 1:
                printf("Insert a value:\n");
                scanf("%d", &value);
                insert_node(&root, value);
                printf("INSERTED!\n");
                break;

            case 2:
                printf("Insert a value to be deleted:\n");
                scanf("%d", &value);
                if (remove_node(&root, value) == 0) {
                    printf("NOT FOUND!\n");
                } else {
                    printf("DELETED!\n");
                }
                break;

            case 3:
                printf("Count nodes: %d\n", count_nodes(root));
                print_inorder(root);
                break;
        }

        printf("\n");

        if (op == 0) {
            printf("Bye!");
            break;
        }

        getchar();
    }

    return (EXIT_SUCCESS);
}