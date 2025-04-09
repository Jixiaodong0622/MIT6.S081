#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"
// 筛选质数，接受一个管道作为参数
void sieve(int pleft[2]){
    // 首先从管道里面读取整数
    int p;
    read(pleft[0], &p, sizeof(p));
    if(p==-1){
        exit(0);// 如果读取到的整数位-1， 表示结束，退出进程
    }
    printf("prime %d\n", p);
    //创建一个管道
    int pright[2];
    pipe(pright);

    // 创建一个进程
    if(fork()==0){ //右邻居
        // 该进程只需要从这个管道里读取数据，不需要往这个管道里面写数据，所以关闭写端
        close(pright[1]);
        close(pleft[0]);
        sieve(pright);
    }else{
        close(pright[0]);
        int buf;
        while(read(pleft[0], &buf, sizeof(buf)) && buf!=-1){
            if(buf % p!=0){
                write(pright[1], &buf, sizeof(buf));
            }
        }
        buf = -1;
        write(pright[1], &buf, sizeof(buf));
        wait(0);
        exit(0);
    }
}

int main(int argc, char* argv[]){
    int input_pipe[2];
    pipe(input_pipe);
    if(fork()!=0){
        close(input_pipe[0]);
        for(int i=2;i<=35;i++){
            write(input_pipe[1], &i, sizeof(i));
        }
        int i =-1;
        write(input_pipe[1], &i, sizeof(i));

    }else{
        // 如果当前进程是子进程
        close(input_pipe[1]);
        sieve(input_pipe);
        exit(0);
    }
    wait(0);
    exit(0);
}