#include "process.h"


int shart_child(int i){

    pid_t pid;
    if( (pid = fork()) < 0 ){
         perror("fork err");
         return 0;
    }else if (pid == 0){
        //in child

#ifdef DEBUG
        printf("child pid %d %d c:%d\n", pid,i,getpid());
#endif
        return i;
    }else{
        //in parent

#ifdef DEBUG
        printf("parent fork child pid %d %d c:%d\n", pid,i,getpid());
#endif
        return 0;
    }

}

void fork_processes(int number){
    int child_id;
    for(int i=1;i<=number;i++) {
          child_id = shart_child(i);
          if(child_id != 0){
              return;
          }
    }

#ifdef DEBUG
    printf("parent wait c:%d\n",getpid());
#endif
    //in parent
    int status;
    pid_t  ex_pid = wait(&status);
    perror("pid return\n");
}

