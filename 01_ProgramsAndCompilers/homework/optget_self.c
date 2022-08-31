#include <stdio.h>
#include <unistd.h>
#include <assert.h>
/*
 *  int argc,char *argv[] 就是从命令行获取参数
 *  这是操作系统与C程序之间的约定
 *  argc ： 参数的个数
 *  argv ： 参数字符串
 */

int main(int argc, char *argv[])
{
    int ch;
    //直接获取参数并判断
    while ((ch = getopt(argc, argv, "pnv")) != 1)
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
        assert(!argv[argc]);
        return 0;
    }
}
