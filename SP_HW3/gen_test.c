#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s filename lines\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    long lines = atol(argv[2]);

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    for (long i = 1; i <= lines; i++) {
        fprintf(fp, "This is line %ld\n", i);
    }

    fclose(fp);
    return 0;
}
