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

process *create_p(const char *, const char *);
int insert(process *, process *, const char *);
int print_tree(process *, int, int *, int);
void listdir(const char *, const int father, process **head, int dep, int flag);

int number_process = 0; // 进程个数

int getPpid(char *filename);
void create_print_pro(int op); //创建树
process *create_p(const char *, const char *);
int build_tree(const char *, process *, int dep);


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

void create_print_pro(int op)
{

    process **head = (process **)malloc(sizeof(process *));
    char buff[1024] = {0};
    listdir(buff, 1, head, 0, op);
    printf("\n");
    int buf[1024] = {0};
    int state_n = 0;
    if (op == 2)
        state_n = 1;
    print_tree((*head), 0, buf, state_n);
    printf("\n");
}

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
        *head = create_p("", "0"); // 初始化

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

void check_memory(void *p)
{
    if (p == NULL)
    {
        perror("malloc error");
        exit(0);
    }
}

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

// tools
int cmp(const char *a, const char *b)
{
    int tempa = atoi(a), tempb = atoi(b);
    if (a == b)
        return 0;
    if (a > b)
        return 1;
    return -1;
}
void insert_node(process *head, process *node)
{
    /*help insert node function*/
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

int insert(process *head, process *node, const char *fpid)
{
    if (node == NULL)
        return -1;
    if (head == NULL)
        return -1;
    while (head)
    {
        if (0 == strcmp(head->pid, fpid))
        {
            if (head->son)
                insert_node(head->son, node);
            else
                head->son = node;
            return 0;
        }
        else
        {
            if (0 == insert(head->son, node, fpid))
                return 0;
        }
        head = head->bro;
    }
    return -1;
}

int print_tree(process *head, int len, int *buf, int flag)
{
    if (head == NULL)
        return -1;

    while (head)
    {
        printf("%s", head->pname);
        if (flag)
            printf("(%s)", head->pid);
        if (head->son)
        {
            if (!head->son->bro)
                printf("\u2500\u2500\u2500");
            else
                printf("\u2500\u252c\u2500");
            int size = strlen(head->pname) + len + 3 + (flag * (strlen(head->pid) + 2));
            if (head->son->bro)
                buf[size - 2] = 1;
            print_tree(head->son, size, buf, flag);
        }
        head = head->bro;
        if (head)
        {
            printf("\n");
            if (!head->bro)
                buf[len - 2] = 0;
            for (int i = 0; i < len - 2; ++i)
                if (buf[i])
                    printf("\u2502");
                else
                    printf(" ");
            if (!head->bro)
                printf("\u2514\u2500");
            else
                printf("\u251c\u2500");
        }
    }
    return 0;
}
