#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include<string.h>
#include <pwd.h>
#include <grp.h>
#include <ctype.h>
uid_t           /* Return UID corresponding to 'name', or -1 on error */
userIdFromName(const char *name)
{
    struct passwd *pwd;
    uid_t u;
    char *endptr;

    if (name == NULL || *name == '\0')  /* On NULL or empty string */
        return -1;                      /* return an error */

    u = strtol(name, &endptr, 10);      /* As a convenience to caller */
    if (*endptr == '\0')                /* allow a numeric string */
        return u;

    pwd = getpwnam(name);
    if (pwd == NULL)
        return -1;

    return pwd->pw_uid;
}

int main(int argc,char *argv[]) {

    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("opendir");
        return 1;
    }

    int uid = 0;
    if(argc>=2)
       uid = userIdFromName( argv[1]); 
    //printf("searching proc run by user with id:%d\n",uid);

    struct dirent *entry;
    errno = 0;
    while ((entry = readdir(dir)) != NULL) {

        char* dname = entry->d_name;
        char* endptr;
        char path[256];
        char line[256];

        strtol(dname, &endptr,10);

        if(*endptr != '\0'){
            //printf("%s is not PID,skip\n",dname);
            continue;
        }

        snprintf(path ,sizeof(path) ,"/proc/%s/status",dname);
        FILE * fp = fopen(path,"r");
        if(!fp){
            printf("Failed to open [%s/]\n",path);
            continue;
        }
        //printf("%s\n",path);
        char pname[256];
        int uidp;
        while(fgets(line,sizeof(line),fp)){
            //printf("%s\n",line);
            if(strncmp(line,"Name:",5) == 0){
                sscanf(line, "Name:%s", pname);
            }
            else if(strncmp(line,"Uid:",4) == 0 ){
                sscanf(line, "Uid:%d", &uidp);
            }
        }
        fclose(fp);
        //printf("%s , %d\n" , pname ,uidp);  
        if(uidp == uid )
            printf("%s\n",pname);

    }

    if (errno != 0) 
        perror("readdir");

    closedir(dir);
    return 0;
}
