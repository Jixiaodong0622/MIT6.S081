#include "kernel/types.h"
#include "user/user.h"
// argc 表示传递的参数个数，argv表示具体的参数
// argv[0] 是 sleep
int main(int argc, char *argv[]){
    if(argc < 2){
        printf("usage: sleep <ticks>\n");
    }
    sleep(atoi(argv[1]));
    exit(0);
}