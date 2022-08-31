#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#define PATH "/proc"
#define BUFFSIZE_INFO 50

typedef struct process
{
    char *pname;
    char *pid;
    struct process *bro;
    struct process *son;
} process;


int main(int argc, char *argv[])
{
    int ch;
    while ((ch = getopt(argc, argv, "pnv")) != -1)
    {
        printf("optind: %d\n", optind);
        switch (ch)
        {
        case 'p':
            printf("HAVE option: -p \n\n");
            create_print_pro(1);
            break;
        case 'n':
            printf("HAVE option: -n \n\n");
            create_print_pro(2);
            break;
        case 'v':
            printf("HAVE option: -v \n\n");
            create_print_pro(3);
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
 * @brief 头节点建立
 * 
 * @param path 
 * @param father 
 * @param head 
 * @param dep 
 * @param flag 
 */
void listdir(const char *path, int father, process **head, int dep, int flag)
{
    DIR *dir_ptr;           // 目录中的句柄
    struct dirent *direntp; //文件结构体
    int pid, ppid;

    dir_ptr = opendir(PATH); //打开目录，并返回句柄
    if (dir_ptr == NULL)
    {
        fprintf(stderr, "can not open /proc\n");
        exit(0);
    }
    if (!dep)
        *head = create_p("os", "0"); // 初始化

    while (direntp = readdir(dir_ptr))
    { //读取句柄，返回dirent结构体
        // printf("%s  %d\n",direntp->d_name,atoi(direntp->d_name));
        pid = atoi(direntp->d_name);
        if (pid)
        {
            if (dep == 1 && pid != father)
                continue;
            build_tree(direntp->d_name, *head, dep);
            if (0 == dep && flag)
            {
                char buf[1024];
                strcpy(buf, "/proc/");
                strcat(buf, direntp->d_name);
                strcat(buf, "/task");
                listdir(buf, pid, head, 1, flag);
            }
        }
    }
    closedir(dir_ptr);
}
/**
 * @brief 获取信息，添加节点
 * 
 * @param path 
 * @param head 
 * @param dep 
 * @return int 
 */
int build_tree(const char *path, process *head, int dep)
{
    const int size = 1024;
    FILE *fp;
    char ch[size];
    strcpy(ch, "/proc/");
    strcat(ch, path);
    strcat(ch, "/status");
    fp = fopen(ch, "r");
    char pid[size], ppid[size], name[size];
    while ((fscanf(fp, "%s", ch)) != EOF)
    {
        if (strcmp(ch, "Pid:") == 0)
        {
            fscanf(fp, "%s", pid);
        }
        if (strcmp(ch, "PPid:") == 0)
        {
            fscanf(fp, "%s", ppid);
        }
        if (strcmp(ch, "Name:") == 0)
        {
            fscanf(fp, "%s", name);
        }
    }
    ch[0] = 0;
    if (dep)
        strcpy(ch, "{");
    strcat(ch, name);
    if (dep)
        strcat(ch, "}");
    insert(head, create_p(ch, pid), ppid);
    fclose(fp);
    return 0;
}
/**
 * @brief 节点初始化，填充获取到信息
 * 
 * @param pname 
 * @param pid 
 * @return process* 
 */
process *create_p(const char *pname, const char *pid)
{
    process *p = (process *)malloc(sizeof(process));
    check_memory(p);
    p->pname = (char *)malloc(sizeof(char) * (strlen(pname) + 1));
    p->pid = (char *)malloc(sizeof(char) * (strlen(pid) + 1));
    check_memory(p->pname);
    check_memory(p->pid);
    strcpy(p->pname, pname);
    strcpy(p->pid, pid);
    p->bro = NULL;
    p->son = NULL;
    return p;
}
void check_memory(void *p)
{
    if (p == NULL)
    {
        perror("malloc error");
        exit(0);
    }
}
/**
 * @brief 
 * 
 * @param head 双亲节点
 * @param node 需要插入的节点
 * @param fpid 
 * @return int 
 */
int insert(process *head, process *node, const char *fpid)
{
    if (node == NULL)
        return -1;
    if (head == NULL)
        return -1;
    while (head)
    {
        if (0 == strcmp(head->pid, fpid))
        {   //head节点是node的父节点    
            
            if (head->son)
            //head有子节点，则node节点将作为head子节点的兄弟节点插入
                insert_node(head->son, node);
            else
            //head没有定义子节点，则node节点将作为head的子节点插入
                head->son = node;
            return 0;
        }
        else
        {
            //head不是node的父节点，head的子节点带入递归
            if (0 == insert(head->son, node, fpid))
                return 0;
        }
        head = head->bro;
    }
    return -1;
}
void insert_node(process *head, process *node)
{
    
    process *cur = head->bro;
    process *prv = head;
    while (cur)
    {
        int state = strcmp(head->pname, node->pname);
        if (1 == state ||
            (0 == state && -1 == cmp(head->pid, node->pname)))
        {
            prv->bro = node;
            node->bro = cur;
            return;
        }
        prv = cur;
        cur = cur->bro;
    }
    prv->bro = node;
    return;
}