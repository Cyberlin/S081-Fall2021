#include "kernel/types.h"
#include "user.h"

int main(int argc,char *argv[]){
    if(argc!=2){
        fprintf(2,"Usage: %s ticks",argv[0]);
        exit(1);
    }else{
        int status;
        int ticks = atoi(argv[1]);
        if((status=sleep(ticks))<0){
            fprintf(2,"can't sleep");
            exit(1);
        }
        exit(0);
    }
}