typedef unsigned int u32;

char *ctable = "0123456789ABCDEF";

int rpu(u32 x, int BASE)
{
    char c;
    if (x){
        c = ctable[x % BASE];
        rpu(x / BASE, BASE);
        putchar(c);
    }
}

int printu(u32 x)
{
    (x==0)? putchar('0') : rpu(x, 10);
    putchar(' ');
}


int printd(int x) {
    if(x < 0) {
        putchar('-');
        x = -x;
    }
    (x==0)? putchar('0') : rpu(x, 10);
    putchar(' ');
}

int printx(u32 x) {
    printf("0x");
    (x==0)? putchar('0') : rpu(x, 16);
    putchar(' ');
}

int printo(u32 x) {
    putchar('0');
    (x==0)? putchar('0') : rpu(x, 8);
    putchar(' ');
}

int prints(char *cp) {
    while ((*cp) != '\0') {
        putchar(*cp);
        cp++;
    }
}

int myprintf(char *fmt, ...) {
    char *cp = fmt;
    int *ip = &fmt;
    ip++;
    while((*cp) != '\0') {
        if ((*cp) == '%') {
            cp++;
            switch (*cp) {
                case 'c':
                    putchar(*ip);
                    putchar(' ');
                    break;
                case 's': ;
                    char *cip = (char*) *ip;
                    prints(cip);
                    break;
                case 'u':
                    printu(*ip);
                    break;
                case 'd':
                    printd(*ip);
                    break;
                case 'o':
                    printo(*ip);
                    break;
                case 'x':
                    printx(*ip);
                    break;
            }
            ip++;
        } else {
            putchar(*cp);
        }
        cp++;
    }
}

int main(int argc, char *argv[ ], char *env[ ]) {
    myprintf("argc= %d, argv= ", argc);
    int i = 0;
    while(argv[i]) {
        myprintf("%s \n", argv[i]);
        i++;
    }
    myprintf("env= ");
    i = 0;
    while(env[i]) {
        myprintf("%s \n", env[i]);
        i++;
    }

}
