#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#define BUFFSIZE_INFO 50
#define ROOT_DIC "/proc"
#define MAX_PROC_NUM 1024
#define MAX_PROC_NAME_LEN 50

struct proInfo
{
    char name[MAX_PROC_NAME_LEN];
    int pid;
    int ppid;
} procs[MAX_PROC_NUM];

int getPname(char *filename);
int getPpid(char *filename);
void setVar();
void show_all();


//进程的个数
int num_procs =0;
char cname[50];

int main(int argc, char *argv[])
{
    setVar();

    
    int ch;
    while ((ch = getopt(argc, argv, "pnv")) != -1)
    {
        printf("optind: %d\n", optind);
        switch (ch)
        {
        case 'p':
            show_all();
            break;
        case 'n':
            printf("HAVE option: -n \n\n");
            break;
        case 'v':
            printf("yyin的pstree -v0.0.1 \n\n");
            break;
        case '?':
            printf("Unknown option: %c\n", (char)optopt);
            break;
        }
    }
    assert(!argv[argc]);
    return 0;
}

void show_all(){
    for (int i = 0; i < num_procs; i++)
    {
        printf("%d %d %s\n",procs[i].pid,procs[i].ppid,procs[i].name);
    }
}

int getPpid(char *filename){
    //打开文件
    FILE *fp = fopen(filename,"r");
    char cppid[50];
    int ppid;
    char buf[500];
    //异常处理
    if (fp == NULL)
    {
        fprintf(stderr, "open file %s error!", filename);
        return -1;
    }
    //读取文件
    while ((fscanf(fp, "%s", buf)) != EOF)
    {
        
        if(strcmp(buf, "PPid:") == 0){
            fscanf(fp, "%s", cppid);
            break;
        }
    }
    ppid = atoi(cppid);
    return ppid;
    
}
int getPname(char *filename){
    //打开文件
    FILE *fp = fopen(filename,"r");
    char pname[50];
    char buf[500];
    //异常处理
    if (fp == NULL)
    {
        fprintf(stderr, "open file %s error!", filename);
        return -1;
    }
    //读取文件
    while ((fscanf(fp, "%s", buf)) != EOF)
    {
        
        if(strcmp(buf, "Name:") == 0){
            fscanf(fp, "%s", pname);
            strcpy(cname,pname);
            break;
        }
    }

    return 0;
    
}

void setVar(){
    int pid;

    //用于拼接路径
    char pidStr[20];
    char proc_path[51] = "/proc/";

    //句柄：一个指向指针的指针保存的是另一个指针的地址，我们可以通过第一个指针来找到另一个指针，然后通过另一个指针访问到具体的数据。
    DIR *dir_ptr;
    struct dirent *dirent;

    dir_ptr=opendir(ROOT_DIC);

    if (dir_ptr == NULL)
    {
        fprintf(stderr, "can not open /proc\n");
        exit(0);
    }

    while (dirent = readdir(dir_ptr))
    { 
        // C 库函数 int atoi(const char *str) 把参数 str 所指向的数字字符串转换为一个整数（类型为 int 型）
        // 如果没有执行有效转换，则返回0，例如文本串
        pid = atoi(dirent->d_name);
        if (pid)
        {
            
            procs[num_procs].pid = pid;

            sprintf(pidStr, "%d", pid);
            strcat(proc_path, pidStr);
            strcat(proc_path, "/status");
            int ppid = getPpid(proc_path); // 返回-1表示出错
            int sname = getPname(proc_path);
            /**
             * 此处ppid的0值作为有效值，不可作为条件判断
             * 计算机内部signed int用补码表示，第一位表示符号位
             * 所以传递-1作为异常值，取反之后，则有~x=-(x+1)，变换为0
             */
            if (~ppid|~sname) //按位取反
            {
                procs[num_procs].ppid = ppid;
                strcpy(procs[num_procs++].name ,cname);
            }
            else
            {
                num_procs++;
            }

            // 重置proc_path
            proc_path[6] = 0;
        }
    }
}