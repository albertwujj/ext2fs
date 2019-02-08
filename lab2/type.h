#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node Node;

typedef struct node {
    char name[64];
    char type;
    Node *childPtr;
    Node *siblingPtr;
    Node *parentPtr;
    Node *prevSibling;
} Node;

Node* searchForDir(Node* curr, char *pname);
