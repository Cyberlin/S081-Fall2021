#include <kernel/types.h>
#include <kernel/stat.h>
#include "kernel/param.h"
#include "user.h"
#define MAX 512

int readline(char** str)
{
    int cnt = 0;
    *str = malloc(MAX * sizeof(char));
    char* p = *str;
    while (read(0, p, 1) != 0) {
        cnt++;
        //printf("%s==%d\n", *str, cnt);
        if (*p == ' ' || *p == '\n') {
            *(p++) = '\0';
            //printf("str==%s\n", *str);
            return cnt;
        } else {
            p++;
        }
    }
    return 0;
}
void run(char** args, int len)
{
    int pid;
    args[len - 1] = 0;
    if ((pid = fork()) == 0) {

        if (exec(args[0], args) < 0) {
            fprintf(2, "Can't exec %s\n", args[0]);
        }
    } else {
        wait((int*)0);
    }
}
int main(int argc, char* argv[])
{
    char* args[MAXARG];
    int i = 0, len = 0;
    //将后面的参数放进来
    for (i = 0; i < argc; i++) {
        args[i] = argv[i + 1];
        len++;
    }
    //printf("i=%d\n", i);
    //当读到EOF 的时候就停止
    while (readline(&args[len - 1]) != 0) {
        len++;

        // if ((pid = fork()) == 0) {

        //     if (exec(args[0], args) < 0) {
        //         fprintf(2, "Can't exec %s\n", args[0]);
        //     }
        // } else {
        //     wait((int*)0);
        // }
    }
    run(args, len);
    exit(0);
}

// #include "kernel/types.h"
// #include "user/user.h"

// int main(int argc, char *argv[]) {
//     char c;
//     char buf[512];
//     uint bi = 0;
//     char *xv[128];
//     int count = 0;
//     for (int i = argc - 2; i < argc; i++) {
//         xv[count] = argv[i];
//         count++;
//     }
//     while (read(0, &c, 1) == 1) {
//         if (c == ' ' || c == '\n') {
//             count++;
//             char *tmp = malloc(bi + 2);
//             memcpy(tmp, buf, bi + 1);
//             tmp[bi + 1] = '\0';
//             xv[count - 1] = tmp;
//             bi = 0;

//             if (c == '\n') {
//                 int f = fork();
//                 if (f < 0) {
//                     fprintf(2, "failed create child process\n");
//                     exit(1);
//                 }
//                 if (f == 0) {
//                     exit(exec(argv[1], xv));
//                 } else {
//                     wait(&f);
//                 }
//                 bi = 0;
//                 count = 0;
//             }
//         } else {
//             buf[bi] = c;
//             bi++;
//         }
//     }
//     exit(0);
// }

// #include "kernel/types.h"
// #include "kernel/param.h"
// #include "user.h"

// int main(int argc, char **argv)
// {
//     char *buf[5];
//     char next;
//     int bp, i;
//     int pid;

//     for (i = 0; i < 5; i++)
//     {
//         buf[i] = malloc(sizeof buf);
//         memset(buf[i], 0, sizeof buf[i]);
//     }

//     for (i = 1; i < argc; i++)
//     {
//         strcpy(buf[i - 1], argv[i]);
//     }
//     bp = 0;
//     while (read(0, &next, 1) != 0)
//     {
//         if (next != '\n')
//         {
//             *(buf[argc - 1] + (bp++)) = next;
//         }
//         else
//         {
//             *(buf[argc - 1] + (bp++)) = '\0';
//             // printf("debug: ");
//             // for (i = 0; i < argc; i++)
//             // {
//             //     printf("%d=%s\n", i, buf[i]);
//             // }

//             pid = fork();
//             if (pid == 0)
//             {
//                 if (exec(buf[0], buf) < 0)
//                 {
//                     fprintf(2, "exec failed\n");
//                 }
//             }
//             else
//             {
//                 wait((int *)0);
//             }
//         }
//     }

//     exit(0);
// }

// xargs.c
// #include "kernel/types.h"
// #include "kernel/stat.h"
// #include "user/user.h"
// #include "kernel/fs.h"

// // 带参数列表，执行某个程序
// void run(char *program, char **args) {
// 	if(fork() == 0) { // child exec
// 		exec(program, args);
// 		exit(0);
// 	}
// 	return; // parent return
// }

// int main(int argc, char *argv[]){
// 	char buf[2048]; // 读入时使用的内存池
// 	char *p = buf, *last_p = buf; // 当前参数的结束、开始指针
// 	char *argsbuf[128]; // 全部参数列表，字符串指针数组，包含 argv 传进来的参数和 stdin 读入的参数
// 	char **args = argsbuf; // 指向 argsbuf 中第一个从 stdin 读入的参数
// 	for(int i=1;i<argc;i++) {
// 		// 将 argv 提供的参数加入到最终的参数列表中
// 		*args = argv[i];
// 		args++;
// 	}
// 	char **pa = args; // 开始读入参数
// 	while(read(0, p, 1) != 0) {
// 		if(*p == ' ' || *p == '\n') {
// 			// 读入一个参数完成（以空格分隔，如 `echo hello world`，则 hello 和 world 各为一个参数）
// 			*p = '\0';	// 将空格替换为 \0 分割开各个参数，这样可以直接使用内存池中的字符串作为参数字符串
// 						// 而不用额外开辟空间
// 			*(pa++) = last_p;
// 			last_p = p+1;

// 			if(*p == '\n') {
// 				// 读入一行完成
// 				*pa = 0; // 参数列表末尾用 null 标识列表结束
// 				run(argv[1], argsbuf); // 执行最后一行指令
// 				pa = args; // 重置读入参数指针，准备读入下一行
// 			}
// 		}
// 		p++;
// 	}
// 	if(pa != args) { // 如果最后一行不是空行
// 		// 收尾最后一个参数
// 		*p = '\0';
// 		*(pa++) = last_p;
// 		// 收尾最后一行
// 		*pa = 0; // 参数列表末尾用 null 标识列表结束
// 		// 执行最后一行指令
// 		run(argv[1], argsbuf);
// 	}
// 	while(wait(0) != -1) {}; // 循环等待所有子进程完成，每一次 wait(0) 等待一个
// 	exit(0);
// }
