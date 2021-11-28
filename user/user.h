struct stat;
struct rtcdate;

// system calls
int fork(void);
int exit(int) __attribute__((noreturn));
int wait(int*);
int pipe(int*);
int write(int, const void*, int);
//从相应的fd读数据到void *  
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
//arg1: 文件名 arg2：权限 return: file descriptor
int open(const char*, int);
//创建一个对于设备的reference
int mknod(const char*, short, short);
int unlink(const char*);
//arg1: file descriptor arg2:stat 结构体 return: inode
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
//进入到子文件
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);

// ulib.c
//arg1: 文件名 arg2:stat 结构体，包含一个文件的相关信息
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void fprintf(int, const char*, ...);
void printf(const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);
int memcmp(const void *, const void *, uint);
void *memcpy(void *, const void *, uint);
