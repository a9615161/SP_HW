#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include<string.h>

#include"getThings.h"

typedef struct childList_{
    int pid;
    struct childList_* next;
}childList;
void addChild(int pid,int ppid,childList** map){

    childList * new = malloc(sizeof(childList));
    new->pid = pid;
    new->next = map[ppid];
    map[ppid] = new;
}
void printTree(int rootPid, childList **cmap, char **nmap, int depth) {
    int layerLen = 15;
    char tmp[256];
    snprintf(tmp, sizeof(tmp), "%s,[%d]", nmap[rootPid], rootPid);
    int dashLen = layerLen - strlen(tmp);

    // 1. 印出自己
    printf("%s", tmp);

    childList *i = cmap[rootPid];

    // 2. 如果沒有子節點 (葉節點)，印出換行並返回
    if (i == NULL) {
        printf("\n");
        return;
    }

    // 3. (有子節點) 為第一個子節點印出橫線
    for (int k = 0; k < dashLen; ++k)
        printf("-");

    // 4. 遞迴呼叫第一個子節點
    printTree(i->pid, cmap, nmap, depth + 1);

    // 5. 迭代處理剩下的兄弟節點
    for (i = i->next; i != NULL; i = i->next) {
        // 5.1 印出前綴空白和連接線
        for (int k = 0; k < layerLen * (1 + depth) - 1; ++k)
            printf(" ");
        printf("-");
        
        // 5.2 遞迴呼叫下一個兄弟節點
        printTree(i->pid, cmap, nmap, depth + 1);
    }
    
    // 6. 拿掉原本在結尾的 printf("\n");
}
#define default_s 65535
int main(){
    unsigned long long pidM = get_pid_max();
    childList ** childMap = calloc(pidM + 1, sizeof(childList*));
    if(childMap == NULL){
        printf("fail to allocate memory of size %llu ,using default size,sth could go wrong\n", (pidM *sizeof(childList*)));
        childMap = calloc(default_s + 1, sizeof(childList*));
    }

    char ** nameMap = calloc(pidM+1,sizeof(char*) );
    if(nameMap == NULL){
        printf("fail to allocate memory of size %llu ,using default size,sth could go wrong\n",(pidM *sizeof(char*)));
        nameMap = calloc(default_s + 1, sizeof(childList*));
    }    
    DIR* dir = opendir("/proc");
    struct dirent *ent ;
    char path[256];

    while( (ent = readdir(dir)) != NULL){
        char* dname = ent->d_name;

        if(!Isnum(dname) )
            continue;
        //printf("%s\n" , dname);
        int pid ,ppid;
        sscanf(dname,"%d",&pid);

        char * PPids = read_status_field(pid,"PPid:");
        char * name = read_status_field(pid,"Name:");
        sscanf(PPids,"%d",&ppid);
        nameMap[pid] = strdup(name);
        addChild(pid,ppid,childMap);
        //char * name = read_status_field(pid,"Name:");
        //printf("%s : %d -> %d\n",name, pid,ppid);

    }
    //in case there are multible init, print from the rootest one
    for(int i = 0;i<pidM;++i)
        if(nameMap[i]!=NULL&&strcmp(nameMap[i],"init") == 0){
            printTree(i,childMap,nameMap,0);
            break;
        }
    
    for(int i = 0;i<pidM;++i){
        free(nameMap[i]);
        free(childMap[i]);
    }
    free(nameMap);
    free(childMap);
}
