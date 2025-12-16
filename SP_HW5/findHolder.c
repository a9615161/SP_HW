#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>     // For opendir, readdir, closedir
#include <unistd.h>     // For readlink

int main(int argc,char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_pathname>\n", argv[0]);
        return 1;
    }
    char target_file[2048] ;
    if (realpath(argv[1], target_file) == NULL) {
        perror("Error resolving path");
        return 1;
    }
    printf("Finding file with path %s\n",target_file);
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("opendir");
        return 1;
    }
    struct dirent *entry;
    //errno = 0;
    while ((entry = readdir(dir)) != NULL) {

        char* dname = entry->d_name;
        char* endptr;
        char path[2048];
        char line[2048];

        strtol(dname, &endptr,10);

        if(*endptr != '\0'){
            //printf("%s is not PID,skip\n",dname);
            continue;
        }
        snprintf(path ,sizeof(path) ,"/proc/%s/fd",dname);

        DIR *fd_dir = opendir(path);
        if (!fd_dir) {
            perror("opendir");
            return 1;
        }    
        struct dirent *fd_entry;
        
        while ((fd_entry = readdir(fd_dir)) != NULL){
            char linkpath[2048];
            char link_tar[2048];
            snprintf(linkpath ,sizeof(linkpath) ,"%s/%s",path,fd_entry->d_name);

            //printf("%s\n",linkpath);
            memset(link_tar,0,sizeof(link_tar));
            int l = readlink( linkpath,link_tar,sizeof(link_tar) );
            if(l == -1)
                continue;
            //printf("%s\n",link_tar);
            if( 0<l && l<=sizeof(link_tar) ){
                link_tar[(l < 2048 ? l : 2048 - 1)] = '\0';
                if(strcmp(target_file,link_tar) == 0){
                    printf("Found! PID: [%s] opened %s\n", dname, target_file);
                    break;
                }
            }
        }
        closedir(fd_dir);
    }
    closedir(dir);
}