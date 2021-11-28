

//测试案例
//echo hello too | xargs echo bye 产生 byte hello too
//echo "1/n2" | xargs -n 1 echo line 产生 line1 和 line2
//find .b | xargs grep hello 产生
//will run "grep hello" on each file named b in the directories below ".".
#include <kernel/types.h>
#include <kernel/stat.h>
#include "kernel/param.h"
#include "user.h"
#define MAX 512
char buf[MAX];
void xargs(int fd, char** argv);
int main(int argc, char* argv[])
{
    char* args[MAXARG];
    int i = 0, pid, flag;
    //int num = 0;
    if ((flag = (argv[1][0] == '-')) != 0) {
        i = 2;
        //num = atoi(argv[2]);
    }

    //成功实现但输入不行
    int temp = i;
    while (i < argc) {
        args[i - temp] = argv[i + 1];
        i++;
    }
    if (flag) {
        i -= 2;
    } else {
        i -= 1;
    }

    int n;
    //当读到EOF 的时候就停止
    while ((n = read(0, buf, sizeof(buf))) > 0) {
        //将读入的\n 去掉
        buf[n-1] = '\0';
        int j = i;

        args[j++] = buf;

        args[j] = 0;
        if ((pid = fork()) == 0) {

            if (exec(args[0], args) < 0) {
                fprintf(2, "Can't exec %s\n", args[0]);
            }
            exit(0);
        } else {
            wait((int*)0);
        }
    }

    

    exit(0);
}
/* 
void xargs(int fd, char** argv)
{
    //先完成不带参数的
    ;
} */
//实验strchr
/* char* buf = "acolin\nbray";
    printf("the buf is %s\n", buf);
    char* p;
    p = strchr(buf, '\n');
    *p = 0;
    p++;
    printf("the buf is %s\n", buf);
    printf("the p is %s\n", p);
 */
/*     //todo read 会读取一个\n
    if ((n = read(0, buf, sizeof(buf))) > 0) {
        printf("the n is %d\n", n);
    }
    //todo 可以使用strchr 来定位\n 从而来分割多个字符串
    buf[n - 1] = '\0';
    printf("the buf is %s\n the size of buf is %d\n\
    the len of bug is %d\n",
        buf, sizeof(buf), strlen(buf));
    args[0] = buf;
    args[1] = "colin";
    args[2] = 0;
    //todo exec（） 成功就不返回
    if (!(res = exec(args[0], args))) {

        
        printf("the res is %d\n", res);

    } else {
        printf("exec success\n and n is %d\n", res);
    } */
/* char* temp = argv[1];
    int cnt = 0, i = 1;
    //xargs 从标准输入读取数据，然后执行相应的命令
    if (argc <= 1) {
        fprintf(2, "Usage: %s <command>", argv[0]);
        exit(1);
    }
    while (temp) {
        cnt += strlen(argv[i]);
        temp = argv[++i];
    }
    printf("the cnt is %d\n", cnt);
    //从标准输入中读取参数，然后执行相应的程序
    //xargs(0, argv);
    exit(0); */