#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * extractF(char* path,const char *field){
    FILE *f = fopen(path, "r");
    if (!f) return NULL;

    char line[256];
    size_t field_len = strlen(field);
    char *result = NULL;

    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, field, field_len) == 0) {
            char *value = line + field_len;
            while (*value == ' ' || *value == '\t') value++;
            value[strcspn(value, "\n")] = '\0'; // 去掉換行符
            result = strdup(value);
            break;
        }
    }
    fclose(f);
    return result;   

}
char *read_status_field(int pid, const char *field) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/status", pid);
    char *result =extractF(path,field);
    return result;
}

int Isnum(char * s){
    char* endptr;
    strtol(s, &endptr,10);
    return *endptr == '\0';
}

int get_pid_max() {
    FILE *f = fopen("/proc/sys/kernel/pid_max", "r");
    if (!f) {
        perror("fopen");
        return 32768; // fallback 預設值
    }
    int pid_max;
    if (fscanf(f, "%d", &pid_max) != 1) {
        fclose(f);
        return 32768; // fallback
    }
    fclose(f);
    return pid_max;
}