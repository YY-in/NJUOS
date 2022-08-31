#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief  Everything is a file
 * all the informations about process is stored in /proc
 * there are many folders named afer "pid"
 * check it and there is one file named "stat",which you can see "ppid" on the four column
 */

#define BUFFSIZE_INFO 50
#define ROOT_DIC "/proc"
#define MAX_PROC_NUM 1024
#define MAX_PROC_NAME_LEN 254

// define a struct to save info of process
struct proInfo
{
    char name[MAX_PROC_NAME_LEN];
    int pid;
    int ppid;
    int floor; // recursive hierarchy
} procs[MAX_PROC_NUM];

int number_process = 0;// 进程个数

int getPpid(char *filename);
void setPid_Ppid();

int main(int argc, char *argv[])
{
    setPid_Ppid();

    for (int i = 0; i < number_process; i++)
    {
        printf("%d %d \n",procs[i].pid,procs[i].ppid);
    }
    int ch;
    while ((ch = getopt(argc, argv, "pnv")) != -1)
    {
        printf("optind: %d\n", optind);
        switch (ch)
        {
        case 'p':
            printf("HAVE option: -p \n\n");
            break;
        case 'n':
            printf("HAVE option: -n \n\n");
            break;
        case 'v':
            printf("HAVE option: -v \n\n");
            break;
        case '?':
            printf("Unknown option: %c\n", (char)optopt);
            break;
        }
    }
    assert(!argv[argc]);
    return 0;
}

/**
 * @brief Get the Ppid object
 *        获取方式：stat中(system) 后第4个参数
 * @param filename
 * @return int
 */
int getPpid(char *filename)
{
    int ppid = -100;
    char *right = NULL;

    FILE *fp = fopen(filename, "r"); // read file
    char info[BUFFSIZE_INFO + 1];
    info[BUFFSIZE_INFO] = '\0'; // string is char[],which is end with '\0'

    if (fp == NULL)
    {
        fprintf(stderr, "open file %s error!", filename);
        return -1;
    }

    /**
     * char *fgets(char *str, int n, FILE *stream)
     * 从指定的流 stream 读取一行，并把它存储在 str 所指向的字符串内。
     * 当读取 (n-1) 个字符时，或者读取到换行符时，或者到达文件末尾时，它会停止，具体视情况而定。
     */
    if (fgets(info, BUFFSIZE_INFO, fp) == NULL)
    {
        puts("fget error!");
        exit(0); // end of process and file-operation
    }

    right = strrchr(info, ')'); //用于检索字符，strchr正向查找，在info所指向的字符串中搜索最后一次出现字符 ')'的位置
    if (right == NULL)
    {
        printf("cannot find )\n");
    }
    right += 3;
    //将right指向的数据格式化传递给ppid
    //int sscanf(const char *str, const char *format, ...)
    sscanf(right, "%d", &ppid);
    return ppid;
}




void setPid_Ppid()
{
    DIR *dir_ptr;           // 目录中的句柄
    struct dirent *direntp; //为了获取某文件夹目录内容，所使用的结构体。
    int pid, ppid;
    char process_path[51] = "/proc/";
    char stat[6] = "/status";
    char pidStr[20];

    dir_ptr = opendir(ROOT_DIC); //打开目录，并返回dir stream
    
    if (dir_ptr == NULL)
    {
        fprintf(stderr, "can not open /proc\n");
        exit(0);
    }

    while (direntp = readdir(dir_ptr))
    { //读取句柄，返回dirent结构体
        // printf("%s  %d\n",direntp->d_name,atoi(direntp->d_name));

        //C 库函数 int atoi(const char *str) 把参数 str 所指向的数字字符串转换为一个整数（类型为 int 型）
        pid = atoi(direntp->d_name);
        if (pid)
        {
            //拼接字符串process_path
            // examples: /proc/pid/stat
            // itoa(pid,pidStr,10);
            procs[number_process].pid = pid;

            sprintf(pidStr, "%d", pid);
            strcat(process_path, pidStr);
            strcat(process_path, stat);
            int ppid = getPpid(process_path); // 返回-1表示出错
            if (~ppid) //按位取反
            {
                procs[number_process++].ppid = ppid;
            }
            else
            {
                number_process++;
            }

            // 重置process_path
            process_path[6] = 0;
        }
    }
}
