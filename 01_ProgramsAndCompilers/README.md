## 目录

*   [哲学](#哲学)

*   [程序就是状态机](#程序就是状态机)

    *   *   *   [状态机视角理解程序运行](#状态机视角理解程序运行)

    *   [C 程序的语义](#c-程序的语义)

        *   [1. 一条特殊指令](#1-一条特殊指令)

        *   [2. 构造最小的hello word](#2-构造最小的hello-word)

    *   [两个视角的切换](#两个视角的切换)

        *   *   [1.什么是编译器？](#1什么是编译器)

            *   [2. 现代(与未来的)编译优化](#2-现代与未来的编译优化)

    *   [操作系统的一般程序](#操作系统的一般程序)

        *   [1.常见应用系统](#1常见应用系统)

        *   [2. 程序类似HelloWorld](#2-程序类似helloworld)

        *   [3. 系统API就是操作系统的全部](#3-系统api就是操作系统的全部)

        *   [4. 各种各样的应用程序](#4-各种各样的应用程序)

*   [作业：打印进程树](#作业打印进程树)

    *   [概述](#概述)

    *   [实验描述](#实验描述)

    *   [提示](#提示)

        *   *   [1.获取命令行参数](#1获取命令行参数)

        *   [2.得到系统中进程的编号](#2得到系统中进程的编号)

        *   [3.得到系统之间的关系](#3得到系统之间的关系)

        *   [4. 建树和打印](#4-建树和打印)

# 01 程序与状态机

![](https://qiniu.yyin.top/01复习.png)

```c
#define REGS_FOREACH(_)  _(X) _(Y)
#define RUN_LOGIC        X1 = !X && Y; \
                         Y1 = !X && !Y;
#define DEFINE(X)        static int X, X##1;
#define UPDATE(X)        X = X##1;
#define PRINT(X)         printf(#X " = %d; ", X);

int main() {
  REGS_FOREACH(DEFINE);
  while (1) { // clock
    RUN_LOGIC;
    REGS_FOREACH(PRINT);
    REGS_FOREACH(UPDATE);
    putchar('\n'); sleep(1);
  }
```

执行`gcc` 或者在vim中输入`:!gcc %`编译,之 后生成可执行文件,运行可执行文件，即可获得状态机得变化

```bash
# root @ aliyun in ~/course/os/01-ProgramsAndCompilers [17:23:32]
$ gcc stateMachine.c

# root @ aliyun in ~/course/os/01-ProgramsAndCompilers [17:23:37]
$ ll
total 24K
-rwxr-xr-x 1 root root 18K Apr  5 17:23 a.out
-rw-r--r-- 1 root root 449 Apr  5 17:21 stateMachine.c
```

利用`gcc -E `的命令查看预编译后的文件，即宏展开

```c
int main() {
  static int X, X1; static int Y, Y1;;
  while (1) {
    X1 = !X && Y; Y1 = !X && !Y;;
    printf("X" " = %d; ", X); printf("Y" " = %d; ", Y);;
    X = X1; Y = Y1;;
    putchar('\n'); sleep(1);
  }
}
```

# 哲学

*   Make each program do one thing well

*   Expect the output of every program to become the input to another

# 程序就是状态机

如果把计算机看成一个状态机, 那么运行在计算机上面的程序又是什么呢?

我们知道程序是由指令构成的, 那么我们先看看一条指令在状态机的模型里面是什么. 不难理解, 计算机正是通过执行指令的方式来改变自身状态的, 比如执行一条加法指令, 就可以把两个寄存器的值相加, 然后把结果更新到第三个寄存器中; 如果执行一条跳转指令, 就会直接修改PC的值, 使得计算机从新PC的位置开始执行新的指令. 所以在状态机模型里面, 指令可以看成是计算机进行一次状态转移的输入激励.

ICS课本的1.1.3小节中介绍了一个很简单的计算机. 这个计算机有4个8位的寄存器, 一个4位PC, 以及一段16字节的内存(也就是存储器), 那么这个计算机可以表示比特总数为`B = 4*8 + 4 + 16*8 = 164`, 因此这个计算机总共可以有`N = 2^B = 2^164`种不同的状态. 假设这个在这个计算机中, 所有指令的行为都是确定的, 那么给定`N`个状态中的任意一个, 其转移之后的新状态也是唯一确定的. 一般来说`N`非常大, 下图展示了`N=50`时某计算机的状态转移图.

![](https://qiniu.yyin.top/01复习.png)

现在我们就可以通过状态机的视角来解释"程序在计算机上运行"的本质了: 给定一个程序, 把它放到计算机的内存中, 就相当于在状态数量为`N`的状态转移图中指定了一个初始状态, 程序运行的过程就是从这个初始状态开始, 每执行完一条指令, 就会进行一次确定的状态转移. 也就是说, 程序也可以看成一个状态机! 这个状态机是上文提到的大状态机(状态数量为`N`)的子集.

例如, 假设某程序在上图所示的计算机中运行, 其初始状态为左上角的8号状态, 那么这个程序对应的状态机为

8->1->32->31->32->31->...

```c
// PC: instruction    | // label: statement
0: addi r1, r2, 2     |  pc0: r1 = r2 + 2;
1: subi r2, r1, 1     |  pc1: r2 = r1 - 1;
2: nop                |  pc2: ;  // no operation
3: jmp 2              |  pc3: goto pc2;
```

#### 状态机视角理解程序运行

以上一小节中`1+2+...+100`的指令序列为例, 尝试画出这个程序的状态机.

这个程序比较简单, 需要更新的状态只包括`PC`和`r1`, `r2`这两个寄存器, 因此我们用一个三元组`(PC, r1, r2)`就可以表示程序的所有状态, 而无需画出内存的具体状态. 初始状态是`(0, x, x)`, 此处的`x`表示未初始化. 程序`PC=0`处的指令是`mov r1, 0`, 执行完之后`PC`会指向下一条指令, 因此下一个状态是`(1, 0, x)`. 如此类推, 我们可以画出执行前3条指令的状态转移过程:

(0, x, x) -> (1, 0, x) -> (2, 0, 0) -> (3, 0, 1)

请你尝试继续画出这个状态机, 其中程序中的循环只需要画出前两次循环和最后两次循环即可.

通过上面必做题的例子, 你应该更进一步体会到"程序是如何在计算机上运行"了. 我们其实可以从两个互补的视角来看待同一个程序:

*   一个是以代码(或指令序列)为表现形式的静态视角, 大家经常说的"写程序"/"看代码", 其实说的都是这个静态视角. 这个视角的一个好处是描述精简, 分支, 循环和函数调用的组合使得我们可以通过少量代码实现出很复杂的功能. 但这也可能会使得我们对程序行为的理解造成困难.

*   另一个是以状态机的状态转移为运行效果的动态视角, 它直接刻画了"程序在计算机上运行"的本质. 但这一视角的状态数量非常巨大, 程序代码中的所有循环和函数调用都以指令的粒度被完全展开, 使得我们难以掌握程序的整体语义. 但对于程序的局部行为, 尤其是从静态视角来看难以理解的行为, 状态机视角可以让我们清楚地了解相应的细节.

```c
void hanoi(int n, char from, char to, char via) {
  if (n == 1) printf("%c -> %c\n", from, to);
  else {
    hanoi(n - 1, from, via, to);
    hanoi(1,     from, to,  via);
    hanoi(n - 1, via,  to,  from);
  }
  return;
}
```

\#include 其实就是实现一个文件上的复制与粘贴&#x20;

```c
#include <stdio.h>
#include "hanoi-r.c"

int main(){
 hanoi(3,'A','B','C');
}
```

C 程序的状态机模型 (语义，semantics)

*   状态 = 堆 + 栈

*   初始状态 = `main` 的第一条语句

*   迁移 = 执行一条简单语句

    *   任何 C 程序都可以改写成 “非复合语句” 的 C 代码

    *   [真的有这种工具](https://cil-project.github.io/cil/ "真的有这种工具") (C Intermediate Language) 和[解释器](https://gitlab.com/zsaleeba/picoc "解释器")

## C 程序的语义

c语言就是整个内存里的东西就是C语言的状态,每执行一条语句,状态就改变一次。当每一条语句执行之，状态改变到最终的状态，就完成了C程序的运行。

C 程序的状态机模型 (语义，semantics)

*   状态 = [stack frame](https://blog.csdn.net/weixin_45511599/article/details/115376231 "stack frame") 的列表 (每个 frame 有 PC程序计数器,存着下一条语句的地址) + 全局变量

*   初始状态 = main(argc, argv), 全局变量初始化

*   迁移 = 执行 top stack frame PC 的语句; PC++

    *   函数调用(入栈) = push frame (frame.PC = 入口)&#x20;

    *   函数返回(出栈) = pop frame

***

应用：将任何递归程序就地转为非递归

*   汉诺塔难不倒你 [hanoi-nr.c](http://jyywiki.cn/pages/OS/2022/demos/hanoi-nr.c "hanoi-nr.c")

```c
#define call(...) ({ *(++top) = (Frame) { .pc = 0, __VA_ARGS__ }; })  //表示是一个可变参数的宏
#define ret()     ({ top--; })
#define goto(loc) ({ f->pc = (loc) - 1; })

void hanoi(int n, char from, char to, char via) {
  Frame stk[64], *top = stk - 1;
  call(n, from, to, via);
  for (Frame *f; (f = top) >= stk; f->pc++) {
    switch (f->pc) {
      case 0: if (f->n == 1) { printf("%c -> %c\n", f->from, f->to); goto(4); } break;
      case 1: call(f->n - 1, f->from, f->via, f->to);   break;
      case 2: call(       1, f->from, f->to,  f->via);  break;
      case 3: call(f->n - 1, f->via,  f->to,  f->from); break;
      case 4: ret();                                    break;
      default: assert(0);
    }
  }
}
```

值得注意的是，状态机并不单纯是线性的，如果我们利用`random`函数可以用于创建状态的分支，理论上可以创建$2^{64}$个分支，这些分支有的可能回到之前状态。

*   所有的指令都只能永远计算

*   deterministic: mov, add, sub, call, ...

*   non-deterministic: rdrand, ...

*   但这些指令甚至都无法使程序停下来 (NEMU: 加条 `trap` 指令)

### 1. 一条特殊指令

调用操作系统syscall,把当前进程所有的状态给都交给操作系统

*   （M,R）完全交给操作系统，任其修改

*   一个有趣的问题：如果程序不打算完全信任操作系统？

*   实现与操作系统中的其他对象交互

    *   读写文件/操作系统状态 (例如把文件内容写入)

    *   改变进程 (运行中状态机) 的状态，例如创建进程/销毁自己

### 2. 构造最小的hello word

&#x20;objdump 查看正常的helloword.c用众多其他内容。

**强行编译 + 链接**：`gcc -c +ld`,结果：直接用ld链接失败，ld不知道怎么链接库函数

**空的main():** 编译成功，但是

*   链接时得到奇怪的警告（可以定义成\_start避免警告）

*   但Segmentation Fault了

**问题：为什么会Segmentation Fault？**

*   我们可以查看程序(状态机)的执行，任何的问题都可以从状态机和状态的转移的角度进行思考

*   GDB yyds

*   `starti` 可以帮助我们从第一条指令开始执行程序

    *   gdb 可以在两种状态机视角之间切换 (`layout asm`)，查看汇编

**有什么办法可以让状态机“停下来”**

*   纯 “计算” 的状态机：不行；要么死循环，要么 undefined behavio

<!---->

*   解决办法：&#x20;

    ```c
    #include <sys/syscall.h>

    int main(){
      //可以调用一个系统调用，并设置返回值42
      syscall(SYS_exit,42);
    }
    ```

Note: gcc 支持对汇编代码的预编译 (还会定义 `__ASSEMBLER__` 宏&#x20;

**回顾：状态机视角程序**

程序 = 计算 → syscall → 计算 → ...

> 💥  彩蛋： ANSI Escape Code 特殊编码实现终端控制

`ssh sshtron.zachlatta.com` (网络游戏)

## 两个视角的切换

我们现在拥有两种状态机视角：

1.  c语言视角

    变量和栈帧(PC)

2.  汇编视角

    内存与寄存器

#### 1.什么是编译器？

**编译器**：源代码  (状态机) → 二进制代码  (状态机)

$$
C=compile(S)
$$

**编译 (优化) 的正确性 (Soundness):**

**S与C  的可观测行为严格一致**

*   system calls; volatile variable loads/stores;termination

*   Trivially 正确 (但低效) 的实现

    *   解释执行/直接翻译  的语义

#### 2. 现代(与未来的)编译优化

在保证观测一致性(sound)的前提下改写代码(rewriting)

*   Inline assambly 也可以参与优化(内联函数)

    *   其他优化可能还会跨过不带barrier的asm volatile

*   系统调用(syscall)时不可优化的,碰到不可优化的代码，则需要保证

    `Eeventual memory consistency`&#x20;

未来的趋势：

*   Semantic-based compilation (synthesis)

*   AI-based rewriting

*   Fine-grained semantics & system call fusion

## 操作系统的一般程序

操作系统中有众多对象，c语言syscall调用系统对象，以此操作系统收编了所有的硬件/软件资源

*   只能用操作系统允许的方式访问操作系统中的对象

    *   从而实现操作系统的 “霸主” 地位

    *   例子：[tryopen.c](http://jyywiki.cn/pages/OS/2022/demos/tryopen.c "tryopen.c")

*   这是为 “管理多个状态机” 所必须的

    *   不能打架，谁有权限就给他

***

### 1.常见应用系统

程序 = 计算+ 系统调用

1.  Core Utilities (coreutils)

    *   *standard* programs for text and file manipulation

    *   系统中安装的是 [GNU Coreutils](https://www.gnu.org/software/coreutils/ "GNU Coreutils")

        *   有较小的替代品 [busybox](https://www.busybox.net/ "busybox")

2.  系统/工具程序

    *   bash, [binutils](https://www.gnu.org/software/binutils/ "binutils"), apt, ip, ssh, vim, tmux, jdk, python, ...

        *   这些工具的原理都不复杂 (例如 apt 其实只是 dpkg 的壳)

        *   [Ubuntu Packages](https://packages.ubuntu.com/ "Ubuntu Packages") (和 apt-file 工具) 支持文件名检索

            *   例子：找不到 `SDL2/SDL.h` 时...

其他各种应用程序

*   浏览器、音乐播放器……

> 杀人面试题

1.  **HelloWorld.c 程序的第一条指令在哪里？= 二进制程序状态机的初始状态是什么？**

    `info proc {mapping}`——打印进程内存 /lib64/ld-linux-x86-64.so.2 加载器

    但是我们可以在elf中进行修改，我们可以更改其他加载器

    所以计算机系统不存在玄学，一切都建立在确定的机制上

2.  **在main执行之前、执行中、执行后，发生了哪些API调用？**

    strace yyds

***

### 2. 程序类似HelloWorld

程序 = 状态机 = 计算 → syscall → 计算 →

*   被操作系统加载

    *   通过另一个进程执行 execve 设置为初始状态

*   状态机执行

    *   进程管理：fork, execve, exit, ...

    *   文件/设备管理：open, close, read, write, ...

    *   存储管理：mmap, brk, ...

*   直到 \_exit (exit\_group) 退出

***

### 3. 系统API就是操作系统的全部

编译器 (gcc)，代表其他工具程序

*   主要的系统调用：execve, read, write

*   `strace -f gcc a.c` (gcc 会启动其他进程)

    *   可以管道给编辑器 `vim -`

    *   编辑器里还可以 `%! 命令` (节/技巧) 对vim 的整个buffer使用命令，替换buffer

图形界面程序 (xedit)，代表其他图形界面程序 (例如 vscode)

*   主要的系统调用：poll, recvmsg, writev

*   `strace xedit`

    *   图形界面程序和 X-Window 服务器按照 X11 协议通信

    *   虚拟机中的 xedit 将 X11 命令通过 ssh (X11 forwarding) 转发到 Host

***

### 4. 各种各样的应用程序

都在操作系统 API (syscall) 和操作系统中的对象上构建

*   窗口管理器

    *   管理设备和屏幕 (read/write/mmap)

    *   进程间通信 (send, recv)

*   任务管理器

    *   访问操作系统提供的进程对象 (readdir/read)

    *   参考 gdb 里的 `info proc *`

*   杀毒软件

    *   文件静态扫描 (read)

    *   主动防御 (ptrace)

    *   其他更复杂的安全机制……

# 作业：打印进程树

## 概述

操作系统能同时运行多个程序。大家都用过[任务管理器](https://www.cyberciti.biz/tips/top-linux-monitoring-tools.html "任务管理器")——能够显示当前系统运行的状态、进程、处理器占用等等。任务管理器，能够显示系统资源的使用情况和进程的信息。当然，“友好地显示” 并不是这门课的主题 (我们假设可以绘制像素点，就能通过代码实现友好的界面)：

作为操作系统课，我们更关注的问题其实是**任务管理器也是操作系统上的应用程序**，我们应该如何实现它？理论上，操作系统就是 “一组对象和操作它们的 API 规约” (例如在 x86-64 系统上，应用程序通过 syscall 指令调用操作系统 API)，所以一定有 API 能获取系统中有哪些正在运行的进程 (和它们的信息，例如使用的内存)。如果你是操作系统的设计者，你会提供怎样的 API (syscall)？

一个可行的想法是操作系统可以提供类似迭代器的 API，可以在某个时刻对进程列表进行 “快照”，然后程序可以通过 API 迭代快照里的进程。

```c
Snapshot *CreateProcessSnapshot(); // 迭代开始
Process *FirstProcess(Snapshot *snapshot); // 取得第一个进程
Process *NextProcess(Process *process); // 获得下一个进程
int ReleaseProcessSnapshot(Snapshot *snapshot); // 迭代结束
```

Windows API 就是这么设计的——没毛病。不过这么做也会使操作系统 API 的数量**暴涨**，因为所有事情都要通过特定的 API 完成——如果考虑 Windows API 动态链接库导出的符号，那么仅 Kernel 和 GDI 的 Windows API 就在 1,000 个以上。

UNIX 操作系统的设计者用另一种方法使应用程序能访问进程列表：提供一个操作系统的对象 (文本文件)，这样应用程序就能用文件 API (open, read, close) 来获取进程列表。例如，创建一个名为 `/system/processes.yaml` 的文件:

就是 everything is a file 的设计。换句话说，我们可以**把操作系统的状态变成文件系统的一部分**——这么做在文件系统里添加了很多对象，但 API 却没有变化。

## 实验描述

Linux 系统自带了 `pstree` 命令，进程树会以非常漂亮的格式排版 (每个进程的第一个孩子都与它处在同一行，之后的孩子保持相同的缩进)：

```xml
systemd─┬─accounts-daemon─┬─{gdbus}
        │                 └─{gmain}
        ├─acpid
        ├─agetty
        ├─atd
        ├─cron
        ├─dbus-daemon
        ├─dhclient
        ├─2*[iscsid]
        ├─lvmetad
        ├─lxcfs───10*[{lxcfs}]
        ├─mdadm
        ├─polkitd─┬─{gdbus}
        │         └─{gmain}
        ├─rsyslogd─┬─{in:imklog}
        │          ├─{in:imuxsock}
        │          └─{rs:main Q:Reg}
        ...
```

Linux 的 psmisc 中 `pstree` 的实现大约有 1,300 行，支持多种命令行参数。这个实验里实现最简单的就行。大家可以先玩一下 Linux 的 `pstree`，使用 `man` 命令查看 `pstree` 支持的功能，并试一试。在这个实验中，我们需要实现它的简化版：`pstree [OPTION]`

**参数描述**

把系统中的进程按照父亲-孩子的树状结构打印到终端。

*   `-p`, `--show-pids`: 打印每个进程的进程号。

*   `-n` `--numeric-sort`: 按照pid的数值从小到大顺序输出一个进程的直接孩子。

*   `-V` `--version`: 打印版本信息。

你可以在命令行中观察系统的 `pstree` 的执行行为 (如执行 `pstree -V`、`pstree --show-pids` 等)。这些参数可能任意组合，但你不需要处理单字母参数合并的情况，例如 `-np`。

**解释**

上述实验要求描述是参照 man page 的格式写出的，其中有很多 UNIX 命令行工具遵守的共同约定 (UNIX 的资深用户对此了如指掌；但对给初学者，尤其是从出生以来就生活在 GUI 环境中而不是遇事就读手册的大家造成了很大的困扰)，例如 [POSIX 对命令行参数有一定的约定](http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html "POSIX 对命令行参数有一定的约定")，**读完以后，你立即发现你对手册的理解增加了** (手册的格式竟然也是 POSIX 标准的一部分！)：

1.  中括号扩起的参数是可选参数，`[]` 后的 `…` 代表参数的 0 次或多次重复。因此 `-p`, `-n`, `-V` 都是可选的参数。

2.  同一个选项可以有不同的名字。在 `pstree` 中，`-p` 和 `--show-pids` 的含义是一样的。

3.  若不另行说明，整数范围在 32 位有符号整数范围内；但如果数值和文件大小有关，则其合法的范围是是 0 到系统最大支持的文件大小。

此外，`main` 函数的返回值代表了命令执行的状态，其中 `EXIT_SUCCESS` 表示命令执行成功，`EXIT_FAILURE` 表示执行失败。对于 POSIX 来说，0 代表成功，非 0 代表失败：例如 `diff`返回 1 表示比较的文件不同，返回 2 表示读取文件失败 (`cmp` 的行为也类似)。UNIX Shell 对返回值有[额外的处理](http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html "额外的处理")。这解释了为什么一些 OJ 会明确要求 main 函数返回值为 0，当返回非 0 时将被认为是 Runtime Error。

## 提示

#### 1.获取命令行参数

<https://www.cnblogs.com/qingergege/p/5914218.html>

```c
#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[]) {
    for (int i = 0; i < argc; i++) {
      assert(argv[i]); // C 标准保证
      printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]); // C 标准保证
    return 0;
}
```

### 2.得到系统中进程的编号

进程是操作系统中的对象，因此操作系统一定提供了 API 访问它们。已经剧透过，系统里的**每个进程都有唯一的编号**，它在 C 语言中的类型是 `pid_t`。不知道这是什么？Google 一把就知道啦。你能找到 glibc 对它的[官方文档解释](https://www.gnu.org/software/libc/manual/html_node/Process-Identification.html "官方文档解释")。以后遇到问题要自己找答案哦！

操作系统以什么样的方式让你获取系统里的进程呢？之前也提示过：

> Everything is a file.

一切皆文件，进程信息当然也可以是 “一切” 的一部分。Linux 提供了 [procfs](https://en.wikipedia.org/wiki/Procfs "procfs")，目录是 `/proc`。如果你进去看一眼，就会发现除了一些比如 `cpuinfo` 这样的文件，还有很多以数字命名的目录——聪明的你也许已经猜到了，每个目录的名字就是进程号，目录里存储了进程相关的运行时数据。

可以先玩一会儿，用 `cat` 可以打印文件的值，可以把文件的内容打出来看看，再对照文档。

现在的问题就变成：怎样得到 `/proc` 目录下的所有以数字为开头的目录。如果你找对关键字，会发现有很多种方法，一定要自己试一试哦。

### 3.得到系统之间的关系

procfs 里还有很多有趣的东西，每个进程的父进程也隐藏在 `/proc/[pid]/` 中的某个文件里。试试 `man 5 proc`，里面有详细的文档。还有，很多 Online Judge 都使用 procfs 读取进程的运行时间/内存数据。

就像一个普通的文件一样，你可以用你熟悉的方式打开 procfs 里的文件：

```c
FILE *fp = fopen(filename, "r");

if (fp) {
  // 用fscanf, fgets等函数读取
  fclose(fp);
} else {
  // 错误处理
}
```

procfs 里的信息足够让你写一个非常不错的任务管理器。也许有同学在实验开始的时候就已经想到——如果你想实现一个任务管理器并且不太会的话，我们可以看一看系统的任务管理器是怎么实现的嘛！我们在课堂上已经演示过 gcc 和 xedit 的例子，就用 strace 工具就能查看进程运行时的系统调用序列：

`strace ps`

```c
...
openat(AT_FDCWD, "/proc/1/stat", O_RDONLY)           = 6
read(6, "1 (systemd) S 0 1 1 0 -1 4194560"..., 1024) = 190
close(6)                                             = 0
...
```

### 4. 建树和打印

这是数据结构方面的内容，这门课不会涉及啦。把它当一个 OJ 题就好了——互联网公司很可能会用类似的题目来考察面试者的基本能力。如果你没有头绪，试着定义一个递归函数 f(T)=\[s1,s2,…,sn] 把 T 打印成多行文本 (第 i 行是字符串 si)。

1.  对于叶子节点，直接输出一个格式化字符串 (例如使用 `asprintf`)；

2.  如果不是叶子节点，对它所有子树 T1,T2,…Tk 分别求 fi(Ti)，得到 k 个多行的文本；

3.  把这些字符串拼到适当的位置，加上一些连接线：
