#include "kernel/types.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"
int dir_count,file_count;

char* fmtname(char *path){
    static char buf[DIRSIZ+1];
    char *p;
    // Find first character after last slash.
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;
    // Return blank-padded name.
    if(strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf+strlen(p), '\0', 1);
    //memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
    return buf;
}

int occur_count(char* path, char target){
    int count = 0;
    for(int i=0;path[i];i++){
        if(path[i] == target){
            count++;
        }
    }
    return count;
}

char* strcat(char *str1, char* str2){
    char* ptr1,*ptr2;
    ptr1 = str1;
    ptr1 += strlen(str1);
    ptr2 = str2;
    ptr2 += strlen(str2);
    strcpy(ptr1,str2);
    return str1;
}


int traverse(char *curdir,char target,int base,int lastflag,char *route){
    char buf[128], *p;
    int root_fd;
    struct dirent de;
    struct stat st;
    if((root_fd = open(route, 0)) < 0){
        fprintf(1, "%s [error opening dir]\n", curdir);
        return 0;
    }
    if(fstat(root_fd, &st) < 0){
        fprintf(2, "ls: cannot stat %s\n", curdir);
        close(root_fd);
        return 0;
    }
    
    int finish = 0; //distinguish directory
    int all_link = 0; //contain . & ..
    switch(st.type){
        case T_FILE:
            fprintf(1, "%s [error opening dir]\n", curdir);
            break;
        case T_DIR:
            if (base == 0){
                printf("%s %d\n",curdir,occur_count(curdir,target));

                base = 1; 
            } 
            if(strlen(route) + 1 + DIRSIZ + 1 > sizeof buf){
                printf("ls: path too long\n");
                break;
            }
            strcpy(buf, route);
            p = buf+strlen(buf);
            *p++ = '/';
            while (read(root_fd, &de, sizeof(de)) == sizeof(de)){
                if(de.inum == 0)
                    continue;
                memmove(p, de.name, DIRSIZ);
                p[DIRSIZ] = 0;
                if(stat(buf, &st) < 0){
                    printf("ls: cannot stat %s\n", buf);
                    continue;
                }
                all_link++;

            }
            close(root_fd);
            finish = 1;
            break;
    }
    if (finish == 1){
        root_fd = open(route, 0);
        int links = 0;
        while (read(root_fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("ls: cannot stat %s\n", buf);
                continue;
            }
            char next[128]; //next file/directory name buffer
            switch(st.type){
                case T_FILE:
                    links++;
                    file_count++;
                    printf("%s %d\n",buf,occur_count(buf,target));
                    break;
                case T_DIR:
                    links++;
                    char new[128];
                    if (links > 2){ 
                        dir_count++;
                    } 
                    strcpy(new,fmtname(buf));

                    if (links > 2) {
                        printf("%s %d\n",buf,occur_count(buf,target));
                    }

                    //processing next path
                    strcpy(next,route);
                    char* temp1;
                    temp1 = strcat(next,"/\0");
                    strcpy(next,temp1);         
                    char *temp;
                    temp = strcat(next,new);
                    strcpy(next,temp);
                
                    //dfs recursive part
                    if (links > 2){
                        if (links == all_link){
                            traverse(new,target,base,1,next);
                        }     
                        else{
                            traverse(new,target,base,0,next);
                        }            
                    }  
                    break;
            }    
        }
        close(root_fd);
    }
    return 0;
}
int worker(char *rootdir,char target){
    int pipefd[2];
    if(pipe(pipefd) < 0){
        printf("pipe error\n");
    }

    int pid;
    if ((pid = fork())< 0){
        fprintf(2,"Tree: fork error\n");
        return 0;
    }
    else if (pid == 0){ // child
        close(pipefd[0]);
        dir_count = 0; file_count = 0;

        traverse(rootdir,target,0,0,rootdir); 
        write(pipefd[1],&dir_count,sizeof(int));
        write(pipefd[1],&file_count,sizeof(int));
        exit(0);
    }
    else { //parent
        close(pipefd[1]);
        wait(0);
        int dir_answer = 0, file_answer = 0;
        read(pipefd[0],&dir_answer,sizeof(int));
        read(pipefd[0],&file_answer,sizeof(int));
        printf("\n");
        printf("%d directories, %d files\n",dir_answer,file_answer);
    }
    return 0;
}
int main(int argc, char *argv[]) {
    if (argc != 3){
        fprintf(2,"Input does not contain one root dir.\n");
        exit(0);
    }
    worker(argv[1],*argv[2]);
    
    exit(0);
}
