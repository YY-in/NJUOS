#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <dirent.h> //Windows中没有这个头文件
#include <unistd.h>

#define MAX_PROC_NUM 1024
#define MAX_PROC_NAME_LEN 254
#define ROOT_FILE "/proc"

struct procInfo
{
    char name[MAX_PROC_NAME_LEN];  //进程的名字
    int pid;        //进程id
    int ppid;       //进程的父进程
    int floor;        //递归的层次
}procs[MAX_PROC_NUM];

int procNum = 0;
int max_floor = 0;

//int atoi(const char *nptr); 把str变成int

void getProcStatus(const char* str,struct procInfo *proc)
{
    FILE *fp;   //获取状态信息
    char t_title[MAX_PROC_NAME_LEN];
    char t_info[MAX_PROC_NAME_LEN];
    fp = fopen(str,"r");

    if(fp == NULL)
    {
        strcpy(proc->name,"NULL");
        proc->ppid = -1;
        proc->floor = -1;
    }
    else
    {
        while( fscanf(fp,"%s",t_title) != EOF )
        {
            if(strncmp(t_title,"Name:",5) == 0)
            {
                fscanf(fp,"%s",proc->name);
            }
            else if(strncmp(t_title,"PPid:",5) == 0)
            {
                fscanf(fp,"%s",t_info);
                proc->ppid = atoi(t_info);
            }
        }
    }

    fclose(fp);
}

void readDirInfo(const char *str)
{
    DIR *dir;
    struct dirent *ptr;
    int tmpLength=0;
    dir = opendir(str); //打开一个目录

    char procStatusString[64];

    //开始获取当前有多少个进程
    while( (ptr = readdir(dir)) != NULL )
    {
        tmpLength = strlen((ptr->d_name));
        int i=0;
        for(i=0;i<tmpLength;i++)
        {
            if((ptr->d_name)[i] <'0' || (ptr->d_name)[i] > '9' )
                break;
        }
        if(i == tmpLength)
        {
            procs[procNum].pid = atoi(ptr->d_name);

            sprintf(procStatusString,"%s/%d/status",ROOT_FILE,procs[procNum].pid);
            //开始获取Status中的信息
            getProcStatus(procStatusString,&(procs[procNum]));
            procNum++;
        }
        if(procNum >= MAX_PROC_NUM)
            break;
    }

    closedir(dir);
}

void GetProcTree(int pid,int step)
{
    for(int i=0;i<procNum;i++)
    {
        if(procs[i].ppid == pid)
        {
            procs[i].floor = step;
            for(int j=0;j<step;j++)
                printf(" ");
            printf("%s\n",procs[i].name);

            GetProcTree(procs[i].pid,step+1);
        }
    }
}

int main()
{
    readDirInfo(ROOT_FILE);
    GetProcTree(0,0);
    printf("procNum = %d\n",procNum);
    return 0;
}

