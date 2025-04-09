#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *path, char *target)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    // 打开目录
    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    // 获取当前目录的信息
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    // 文件、目录分别处理方式
    switch (st.type)
    {
    case T_FILE: // 如果是文件，应该与target文件名进行对比
        if (strcmp(path + strlen(path) - strlen(target), target) == 0)
        {
            printf("%s\n", path);
        }
        break;
    case T_DIR: // 如果是一个目录，我们应该递归调用find，在子目录
        // 判断path添加完目录最大长度是否超出限制
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            printf("find: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(path);
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            // de表示该目录下的一个目录项，可能是一个文件，也可能是一个子目录
            if (de.inum == 0)
            { // 如果文件的inode为0
                continue;
            }
            memmove(p, de.name, DIRSIZ); // 把文件名拼在path后面形成完整路径
            p[DIRSIZ] = 0;               // 构成合法的字符串，与 '\0' 等价
            if (stat(buf, &st) < 0)
            { // 获取当前文件或目录的元信息，并放入st中，如成功则返回0
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            // 首先要排除 "."和".."两个目录
            if (strcmp(buf + strlen(buf) - 2, "/.") != 0 && strcmp(buf + strlen(buf) - 3, "/..") != 0)
            {
                find(buf, target);
            }
        }
        break;
    }
    close(fd);
}
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        exit(0);
    }
    char target[512];
    target[0] = '/';
    strcpy(target + 1, argv[2]);
    find(argv[1], target);
    exit(0);
}