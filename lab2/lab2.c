#include "type.h"

Node *root, *cwd; // root and CWD pointers
char dname[64], bname[64]; // dirname and basename string holders
char *cmds[] = {"mkdir", "rmdir", "ls", "cd", "pwd", "creat", "rm",
    "reload", "save", "menu", "quit", NULL};

int insertNode(Node *newDir, Node *parent) {
    //insert newDir as first child of parent
    newDir->parentPtr = parent;
    newDir->siblingPtr = parent->childPtr;
    parent->childPtr = newDir;
    if (newDir->siblingPtr != NULL) {
        newDir->siblingPtr->prevSibling = newDir;
    }
}

int findCmd(char *command) {

    char **curr = cmds;
    int i = 0;
    while (*curr != NULL) {
        if(strcmp(*curr, command) == 0) {
            return i;
        }
        curr++;
        i++;
    }
    return -1;
}

int mk(char *pathname, char filetype) {
    //split into dname and bname by finding last slash
    char *c = pathname;
    char *lastslash = NULL;
    while (*c != '\0') {
        if (*c == '/') {
            lastslash = c;
        }
        c++;
    }
    Node *parent = NULL;
    if (lastslash == NULL) {
        strcpy(dname, "");
        strcpy(bname, pathname);
    }
    else {
        *(lastslash) = '\0';
        strcpy(dname, pathname);
        strcpy(bname, lastslash + 1);
    }

    //find parent with dname
    if (*dname == '/') {
        parent = searchForDir(root, dname+1);
    }
    else {
        parent = searchForDir(cwd, dname);
    }

    if (parent == NULL || parent->type == 'F') {
        return 1;
    }
    //check if node with name already exists in parent's children
    Node *checkDup = parent->childPtr;
    while (checkDup != NULL) {
        if (strcmp(checkDup->name, bname) == 0) {
            return 1;
        }
        checkDup = checkDup->siblingPtr;
    }
    //create and insert new node
    Node *newDir = malloc(sizeof(Node));
    strcpy(newDir->name, bname);
    insertNode(newDir, parent);
    newDir->type = filetype;
    return 0;
}

int rm(char *pathname, char filetype) {
    //find node with pathname
    Node *curr = NULL;
    if (*pathname == '/') {
        curr = searchForDir(root, pathname+1);
    }
    else {
        curr = searchForDir(cwd, pathname);
    }
    if (curr == NULL || curr->type != filetype) {
        return 1;
    }

    //if file or empty directory, remove
    if(filetype == 'F' || curr->childPtr == NULL) {
        Node *parent = curr->parentPtr;
        if(parent == NULL) {
            printf("Tried to delete root");
            return 1;
        }
        //remove temp
        Node *temp = curr;
        if(curr->prevSibling == NULL) {
            parent->childPtr = curr->siblingPtr;
            if (curr->siblingPtr != NULL) {
                curr->siblingPtr->prevSibling = NULL;
            }
        } else {
            if (curr->siblingPtr != NULL) {
                curr->siblingPtr->prevSibling = curr->prevSibling;
            }
            curr->prevSibling->siblingPtr = curr->siblingPtr;
        }
        free(temp);
        return 0;
    }
    return 1;
}

int ls(char *pathname) {
    //find parent with pathname
    Node *parent = NULL;
    if (*pathname == '/') {
        parent = searchForDir(root, pathname+1);
    }
    else {
        parent = searchForDir(cwd, pathname);
    }
    if (parent == NULL || parent->type != 'D') {
        return 1;
    }

    //print children
    Node *curr = parent->childPtr;
    if (curr == NULL) {
        printf("No files in dir\n");
    }
    while(curr != NULL) {
        printf("[%c %s] ", curr->type, curr->name);
        curr = curr->siblingPtr;
    }
    return 0;
}
int cd(char *pathname) {
    //find node with pathname
    Node *curr = NULL;
    if (*pathname == '/') {
        curr = searchForDir(root, pathname+1);
    }
    else {
        curr = searchForDir(cwd, pathname);
    }
    if (curr == NULL || curr->type != 'D') {
        return 1;
    }

    cwd = curr;
    return 0;
}
void pwd(Node *curr) {
    if (curr == NULL) {
        return;
    }
    pwd(curr->parentPtr);
    printf("/%s", curr->name);
    return;
}

void savehelper(FILE *out, Node *curr, char *fullPath, char *end) {
    if(curr == NULL) {
        return;
    }

    //append curr node name to current path
    *end = '/';
    char *d = end + 1;
    char *source = curr->name;
    while (*source != '\0') {
        *d = *source;
        d++;
        source++;
    }
    *d = '\0';

    fprintf(out, "%c %s\n", curr->type, fullPath);
    //preorder recurse
    savehelper(out, curr->childPtr, fullPath, d);
    *end = '\0';
    savehelper(out, curr->siblingPtr, fullPath, end);
}

int save(char *filename) {
    FILE *out = fopen(filename, "w");
    if (out == NULL) {
        return 1;
    }
    char start[64];
    savehelper(out, root, start, start);
    return 0;
}


void reloadhelper(Node *prev, Node *parent, FILE *file, char *curr, char *currslash, int isChild) {
    //remove newline from curr(path)
    currslash[strlen(currslash) - 1] = 0;

    //create new node
    Node *node = malloc(sizeof(Node));
    node->type = *curr;
    strcpy(node->name, currslash + 1); //curr slash is the final slash

    //insert node after prev as child or sibling
    if (isChild) {
        prev->childPtr = node;
        node->parentPtr = prev;
    } else {
        prev->siblingPtr = node;
        node->prevSibling = prev;
    }

    //save file position, read next line
    char next[66];
    int orig = ftell(file);
    if (fgets(next,66,file) == NULL) { return; }

    char *nextslash = currslash - curr + next; // calculates curr slash pos in next line

    //checks if next line is child by detecting if there is slash after nextslash
    char *checkchild = nextslash + 1;
    while (*checkchild != '\0' && *checkchild != '/') {
        checkchild++;
    }

    if (*checkchild == '/') {
        //recurse on child
        reloadhelper(node, node, file, next, checkchild, 1);
        orig = ftell(file);
        if (fgets(next,66,file) == NULL) { //get next line for siblings
            return;
        }
    }
    nextslash = currslash - curr + next;

    char *name = parent->name;
    int name_length = 0;
    while (*name != '\0') {
        name++;
        name_length++;
    }

    //check if next line is sibling by checking parent name match
    if (strncmp(nextslash - (name_length * sizeof(char)), parent->name, name_length) != 0) {
        //next line is not sibling but higher up on tree, return file to orig position
        fseek(file, orig, 0);
        return;
    }
    //recurse on sibling
    reloadhelper(node, parent, file, next, nextslash, 0);
}


int reload(char *filename) {
    FILE *in = fopen(filename, "r");
    if (in == NULL) {
        return 1;
    }
    char curr[66]; //66 = path max chars + file type char + space
    if (fgets(curr,66,in) == NULL) { return 0;}
    Node dummys;
    Node *dummy = &dummys;
    reloadhelper(dummy, dummy, in, curr, curr + 2, 1);
    root = dummy->childPtr;
    root->parentPtr = NULL;
    cwd = root;
    return 0;
}

void menu() {
    printf("Commands:\n");
    char **cmdptr = cmds;
    while(*cmdptr != NULL) {
        printf("%s\n", *cmdptr);
        cmdptr++;
    }
}

Node* searchForDir(Node* curr, char *pname){
    if(curr == NULL) {
        printf("Node not found");
        return NULL;
    }
    if (strcmp(pname, "") == 0) { //reached end of filename
        return curr;
    }
    char *c = pname;
    while(*c != '/' && *c != '\0') {
        c++;
    }
    int end = (*c == '\0');
    *c = '\0';
    if (strcmp(pname, "..") == 0) { //go up
      return searchForDir(curr->parentPtr, c + 1);
    }
    if (strcmp(pname, ".") == 0) { //do nothing
      return searchForDir(curr, c + 1);
    }
    curr = curr->childPtr;
    while(curr != NULL && strcmp(curr->name, pname) != 0) {
        curr = curr->siblingPtr;
    }
    if (end) { return curr; }
    return searchForDir(curr, c + 1);
}

int initialize() {
    root = malloc(sizeof(Node));
    strcpy(root->name, "");
    root->type = 'D';
    cwd = root;
}

void deleteTree(Node *curr) {
    if (curr == NULL) {
        return;
    }
    Node *child = curr->childPtr;
    Node *sibling = curr->siblingPtr;
    free(curr);
    deleteTree(child);
    deleteTree(sibling);
}

int main()
{
    int index;
    char line[128], command[16], pathname[64];
    initialize(); //initialize root node of the file system tree
    while(1){
        command[0] = '\0';
        pathname[0] = '\0';
        line[0] = '\0';
        printf("input a command line: ");
        fgets(line,128,stdin);
        line[strlen(line)-1] = 0;
        sscanf(line, "%s %s", command, pathname);
        index = findCmd(command);
        int fail = 0;
        switch(index){
            case 0:
                fail = mk(pathname, 'D'); break;
            case 1:
                fail = rm(pathname, 'D'); break;
            case 2:
                fail = ls(pathname); break;
            case 3:
                fail = cd(pathname); break;
            case 4:
                pwd(cwd); printf("\n"); break;
            case 5:
                fail = mk(pathname, 'F'); break;
            case 6:
                fail = rm(pathname, 'F'); break;
            case 7:
                fail = reload(pathname); break;
            case 8:
                fail = save(pathname); break;
            case 9:
                menu(); break;
            case 10:
                save("tree.txt");
                deleteTree(root);
                exit(0);
            default: printf("invalid command %s\n", command);
        }
        if(fail) {
            printf("command %s failed\n", command);
        }
    }
}
