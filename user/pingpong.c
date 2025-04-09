#include "kernel/types.h"
#include "user/user.h"
#include "kernel/stat.h"

int main(){
    // 创建父进程的两个文件标识符，用于读写管道里面的数据，父进程通过pp2c[1]往管道里面写数据，子进程通过pp2c[0]从管道里读数据
    // 创建子进程的两个文件标识符，用于读写管道里面的数据
    // fd[0]用于读管道，fd[1]用于写管道
    int pp2c[2], pc2p[2];
    pipe(pp2c);
    pipe(pc2p);
    if(fork()!=0){
        // 此时为父进程，父进程向子进程发送一个字符
        write(pp2c[1], ".", 1);
        close(pp2c[1]);// 之所以将写端关闭，是因为如果不关闭，当管道中数据为空时，读端会堵塞

        // 父进程从管道读取子进程读取字符
        char buf;
        read(pc2p[0], &buf, 1);
        printf("%d: received pong\n", getpid());
        wait(0);// 等待子进程结束
    }else{
        // 此时为子进程，子进程先读取父进程发送的字符
        char buf;
        read(pp2c[0], &buf, 1);
        printf("%d: received ping\n", getpid());

        // 子进程向父进程发送字符
        write(pc2p[1], ".", 1);
        close(pc2p[1]);
    }
    close(pc2p[0]);
    close(pp2c[0]);
    exit(0);
}