#include<stdlib.h>
#include<stdio.h>
int main(){
    printf("hostname:");
    fflush(stdout);
    system("hostname");

    system("uname -r");

    printf("hostid:");
    fflush(stdout);
    system("hostid");

    return 0;
}
