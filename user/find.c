// #include "kernel/types.h"
// #include "kernel/fcntl.h"
// #include "kernel/fs.h"
// #include "kernel/stat.h"
// #include "user.h"

// char *
// fmtname(char *path)
// {
//     char *p;

//     for (p = path + strlen(path); p >= path && *p != '/'; p--)
//         ;
//     p++;

//     return p;
// }

// void find(char *path, const char *name)
// {
//     struct dirent de;
//     struct stat st;
//     char buf[512], *b;
//     int fd;

//     fd = open(path, O_RDONLY);
//     if (stat(path, &st) < 0)
//     {
//         fprintf(2, "stat failed for %s\n", path);
//         exit(1);
//     }

//     switch (st.type)
//     {
//     case T_FILE:
//         b = fmtname(path);
//         // printf("%s,%s,%s\n", name, b, path);
//         if (strcmp(b, name) == 0)
//         {
//             printf("%s\n", path);
//         }
//         break;
//     case T_DIR:
//         strcpy(buf, path);
//         b = buf + strlen(buf);
//         *b++ = '/';
//         // printf("b1 = %s\n", buf);
//         while (read(fd, &de, sizeof de) == sizeof de)
//         {
//             if (de.inum == 0)
//                 continue;

//             if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
//                 continue;

//             memmove(b, de.name, DIRSIZ);
//             b[DIRSIZ] = 0;
//             // printf("b2 = %s\n", buf);
//             find(buf, name);
//         }
//         break;
//     }
//     close(fd);
// }

// int main(int argc, char **argv)
// {
//     if (argc < 3)
//     {
//         fprintf(2, "usage: find [path] [pattern]\n");
//         exit(1);
//     }
//     find(argv[1], argv[2]);

//     exit(0);
// }
// //大概率是使用stat 和open 和read
// //去看下ls 的代码
// //这玩意怎么debug
// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "user.h"
// #include "kernel/fs.h"
// #define MAX 512

// void find(char* path, char* pat);
// char* fmtname(char* path)
// {
//     char* p;

//     for (p = path + strlen(path); p >= path && *p != '/'; p--)
//         ;
//     p++;

//     return p;
// }
// int main(int argc, char* argv[])
// {
//     //如果参数小于2,也就是没有参数的话，使用ls
//     if (argc < 2) {
//         find(".", ".");
//         exit(0);
//     }
//     if (argc == 3) {
//         find(argv[1], argv[2]);
//         exit(0);
//     }
//     fprintf(2, "Usage: %s / /<dir> <patern>\n");
//     exit(0);
// }
// //遍历当前目录，然后向下查找，直到没有相应的文件和目录
// void find(char* path, char* pat)
// {
//     int fd;
//     struct stat st;
//     char buf[MAX];
//     struct dirent de;
//     char* p;
//     if ((fd = open(path, 0)) < 0) {
//         fprintf(2, "find: cannot open %s\n", path);
//         exit(1);
//     }
//     if (fstat(fd, &st) < 0) {
//         fprintf(2, "find: cannot open %s\n");
//         exit(1);
//     }
//     switch (st.type) {
//     case T_FILE:
//         if (strcmp(fmtname(path), pat) == 0) {
//             printf("%s\n", path);
//         }
//         //printf("%s\t%d %d %l\n", path, st.type, st.ino, st.size);
//         break;
//     case T_DIR:
//         if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
//             printf("find: path too long\n");
//             break;
//         }
//         strcpy(buf, path);
//         p = buf + strlen(path);
//         *p++ = '/';
//         //每次处理一个目录项
//         //todo 这里read 为什么可以总是转成其他的类型的
//         while (read(fd, &de, sizeof(de)) == sizeof(de)) {
//             if (de.inum == 0) {
//                 //todo inum 等于零是什么意思
//                 continue;
//             }
//             //从index:n-1开始到0结束,一共n个字节
//             memmove(p, de.name, DIRSIZ);
//             //等于零是因为转化成字符串
//             p[DIRSIZ] = 0;
//             if (stat(buf, &st) < 0) {
//                 printf("find: cannot stat %s\n", buf);
//                 continue;
//             }
//             if (strcmp(p, ".") != 0 && strcmp(p, "..") != 0)
//                 find(buf, pat);
//             //printf("%s %d %d %d\n", buf, st.type, st.ino, st.size);
//         }
//         break;

//     default:
//         break;
//     }
//     close(fd);
// }

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *dir, char *file)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(dir, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", dir);
        return;
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", dir);
        close(fd);
        return;
    }

    if (st.type != T_DIR)
    {
        fprintf(2, "find: %s is not a directory\n", dir);
        close(fd);
        return;
    }

    if(strlen(dir) + 1 + DIRSIZ + 1 > sizeof buf)
    {
        fprintf(2, "find: directory too long\n");
        close(fd);
        return;
    }

    strcpy(buf, dir);
    p = buf+strlen(buf);
    *p++ = '/';

    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        if(de.inum == 0)
            continue;
        if (!strcmp(de.name, ".") || !strcmp(de.name, ".."))
            continue;

        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0)
        {
            fprintf(2, "find: cannot stat %s\n", buf);
            continue;
        }

        if (st.type == T_DIR)
        {
            find(buf, file);
        }
        else if (st.type == T_FILE && !strcmp(de.name, file))
        {
            printf("%s\n", buf);
        } 
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(2, "usage: find dirName fileName\n");
        exit(1);
    }

    find(argv[1], argv[2]);
    exit(0);
}
