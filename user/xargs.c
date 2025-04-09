#include "kernel/types.h"
#include "user/user.h"

void run(char *program, char **args)
{
    if (fork() == 0)
    {
        exec(program, args);
        exit(0);
    }
    return;
}

int main(int argc, char *argv[])
{
    char buf[2048];
    char *p = buf, *last_p = buf;
    char *argsbuf[128];
    char **args = argsbuf;

    // 首先把xargs的参数复制到 argsbuf 中
    for (int i = 1; i < argc; i++)
    {
        *args = argv[i]; // args存放第 i 个参数的的首地址
        args++;
    }
    // 记录当前参数的位置
    char **pa = args;

    // 从标准输入里面读取数据，存储到缓冲区buf中。就相当于将xargs之前的指令输出放进管道，xargs再从管道里面读取，放置尾部
    while (read(0, p, 1) != 0)
    {
        if (*p == ' ' || *p == '\n')
        {
            // 使用p遍历缓冲区，如果遇到空格或者换行，将其替换成'\0'
            *p = '\0';

            *(pa++) = last_p;
            last_p = p + 1;

            // 如果遇到换行符，表示一组参数读完，需要开始执行程序
            if (*p == '\n')
            {
                *pa = 0;
                run(argv[1], argsbuf);
                pa = args;
            }
        }
        p++;
    }

    if (pa != args)
    {
        *p = '\0';
        *(pa++) = last_p;
        *pa = 0;

        run(argv[1], argsbuf);
    }

    while (wait(0) != -1)
    {
    };
    exit(0);
}