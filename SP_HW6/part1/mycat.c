#include<stdio.h>
#include<stdlib.h>
#include <errno.h> // For better error reporting (using perror)

int main(int argc,char * argv[]){
    if (argc != 2) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return 1;
    }

    const char *fname = argv[1]; 
    FILE * f = fopen(fname, "r");
    if(f == NULL){
        perror(fname);
        return 1;
    }
    char c;
    while( (c = fgetc(f) ) != EOF)
        putchar(c);
    fclose(f);

    return 0;
}