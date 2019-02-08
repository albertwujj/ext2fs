#include <stdio.h>
#include <stdlib.h>

int *FP;

int main(int argc, char *argv[ ], char *env[ ])
{
    int a,b,c;
    printf("enter main\n");

    printf("&argc=%x argv=%x env=%x\n", &argc, argv, env);
    printf("&a=%8x &b=%8x &c=%8x\n", &a, &b, &c);

    a=1; b=2; c=3;
    A(a,b);
    printf("exit main\n");
}

int A(int x, int y)
{
    int d,e,f;
    printf("enter A\n");
    // write C code to PRINT ADDRESS OF d, e, f
    printf("&d=%p &e=%p &f = %p\n", &d, &e, &f);
    d=4; e=5; f=6;
    B(d,e);
    printf("exit A\n");
}

int B(int x, int y)
{
    int g,h,i;
    printf("enter B\n");
    // write C code to PRINT ADDRESS OF g,h,i
    printf("&g=%p &h=%p &i=%p\n", &g, &h, &i);
    g=7; h=8; i=9;
    C(g,h);
    printf("exit B\n");
}

int C(int x, int y)
{
    int u, v, w, i, *p;

    printf("enter C\n");
    // write C cdoe to PRINT ADDRESS OF u,v,w,i,p;
    printf("&u=%p &v=%p &w=%p &i=%p &p=%p\n",&u,&v,&w,&i,&p);
    u=10; v=11; w=12; i=13;

    FP = (int *)getebp();  // FP = stack frame pointer location of the C() function

    printf("printing stack frame linked list:\n");
    int *currfp = FP;
    while (*currfp != 0) {
        printf("%p\n", currfp);
        currfp = *currfp;
    }
    printf("%p\n", currfp); //crt0's stack frame pointer's location

    printf("printing stack contents from &p up to crt0's stack frame pointer\n");
    p = (int *)&p;
    for(p; p < FP; p++) { //print from &p up to C's stack frame pointer
        printf("%p: %p\n", p, *p);
    }
    for(p; p <= currfp + 1; p++) { //print from C's stack frame pointer up to crt0's stack frame pointer
        printf("%p: %p\n", p, *p);
    }
    printf("exit C\n");
}
