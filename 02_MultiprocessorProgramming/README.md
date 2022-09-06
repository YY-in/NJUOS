## 目录

*   [复习](#复习)

*   [入门](#入门)

    *   [并发](#并发)

        *   [并发的基本单位：线程](#并发的基本单位线程)

        *   [简化的API线程](#简化的api线程)

            *   [一个简单的多线程程序](#一个简单的多线程程序)

            *   [POSIX Threads](#posix-threads)

*   [放弃](#放弃)

    *   [原子性](#原子性)

        *   [原子性的丧失](#原子性的丧失)

        *   [线程安全](#线程安全)

        *   [实现原子性](#实现原子性)

    *   [顺序](#顺序)

        *   [例子：求和 (再次出现)](#例子求和-再次出现)

        *   [顺序的丧失](#顺序的丧失)

        *   [实现源码顺序翻译](#实现源码顺序翻译)

    *   [可见性](#可见性)

        *   [例子](#例子)

        *   [现代处理器：处理器也是 (动态) 编译器！](#现代处理器处理器也是-动态-编译器)

        *   [多处理器间即时可见性的丧失](#多处理器间即时可见性的丧失)

*   [总结](#总结)

    *   [总结](#总结-1)

# 02 多处理器编程

# 复习

应用程序的视角的操作系统 = syscall 指令

**本次课回答的问题：**

在多处理器时代，如何理解状态机模型？

# 入门

使用多线程的原因：

1.  &#x20;Parallelism

2.  To avoid blocking program progress due to slow I/O

Human is sequential creaeor！

Coding  get world！

## 并发

> Concurrent:  existing,happening,or done at the same time.
>
> > In computer science,concurrency refers to the ability of different parts or units of a program,algorithm,or problem to be executed out-of-order or in partial order,without affecting the final outcome. &#x20;

我们的社会天生就是并发的。程序分成好几个部分，这几个部分会已任意的顺序执行。

系统调用的代码是世界上最早的并发程序。

### 并发的基本单位：线程

共享内存的多个执行流：

*   执行流拥有独立的堆栈/寄存器

*   共享全部的内存(指针可以互相引用)

**把单线程的计算机改造成多线程的计算机：**

并发程序执行的每一步都是不确定

我们的全局状态，包括全局变量和堆区需要成为共享

但线程都应该拥有自己独立的局部变量和PC，即要有属于自己的栈帧链

我们可以简化并发系统的状态机，是每次选择一个线程执行一步语句，每次执行之后，会产生全新的全局变量和对应线程的局部状态，而其他线程的局部状态不变

### 简化的API线程

```c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdatomic.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#define NTHREAD 64
enum { T_FREE = 0, T_LIVE, T_DEAD, };
struct thread {
  int id, status;
  pthread_t thread;
  void (*entry)(int);
};

struct thread tpool[NTHREAD], *tptr = tpool;

void *wrapper(void *arg) {
  struct thread *thread = (struct thread *)arg;
  thread->entry(thread->id);
  return NULL;
}

void create(void *fn) {
  assert(tptr - tpool < NTHREAD);
  *tptr = (struct thread) {
    .id = tptr - tpool + 1,
    .status = T_LIVE,
    .entry = fn, 
  };  
  pthread_create(&(tptr->thread), NULL, wrapper, tptr);
  ++tptr;
}

void join() {
  for (int i = 0; i < NTHREAD; i++) {
    struct thread *t = &tpool[i];
    if (t->status == T_LIVE) {
      pthread_join(t->thread, NULL);
      t->status = T_DEAD;
    }   
  }
}

__attribute__((destructor)) void cleanup() {
  join();
}

```

jyy为大家封装了超级好用的线程 API ([thread.h](http://jyywiki.cn/pages/OS/2022/demos/thread.h "thread.h"))

*   `create(fn)`

    *   创建一个入口函数是 `fn` 的线程，并立即开始执行

        *   `void fn(int tid) { ... }`

        *   参数 `tid` 从 1 开始编号

    *   语义：在状态中新增 stack frame 列表并初始化为 `fn(tid)`

    *   解释：假设原先我们拥有某台状态机，该状态机只有一个线程。执行`create(fn)`就相当于：

        1.  function执行完之后，原来线程的PC→PC+1，且栈帧仍旧存在

        2.  create(fn)返回以后，就会创建一个新的执行流，新的执行流PC=0，PC指向新函数，函数里有tid=xxx(线程号)，此时PC指向的就是fn函数

*   `join()`

    *   等待所有运行线程的 `fn` 返回

    *   在 `main` 返回时会自动等待所有线程结束

    *   语义：在有其他线程未执行完时死循环，否则返回

    *   解释：假设状态机里已经有多个线程t1,t2,t3。假设t1执行`join()`，它就进入了一个死循环:

        *   当t2或t3任意一个没有执行结束时，这个时候再执行t1，t1就会不断回到同一个状态

        *   当t2和t3都执行结束之后，这时候只剩下t1了，t1执行join()

*   编译时需要增加 `-lpthread`

    > 编译程序包括 预编译， 编译，汇编，链接，包含头文件了，仅能说明有了线程函数的声明， 但是还没有实现， 加上-lpthread是在链接阶段，链接这个库。
    >
    > pthread是动态库，需要用-lpthread，所有的动态库都需要用-lxxx来引用用gcc编译使用了POSIX thread的程序时通常需要加额外的选项，以便使用thread-safe的库及头文件，一些老的书里说直接增加链接选项 -lpthread 就可以了而gcc手册里则指出应该在编译和链接时都增加 -pthread 选项编译选项中指定 -pthread 会附加一个宏定义 -D\_REENTRANT，该宏会导致 libc 头文件选择那些thread-safe的实现；
    >
    > 链接选项中指定 -pthread 则同 -lpthread 一样，只表示链接 POSIX thread 库。由于 libc 用于适应 thread-safe 的宏定义可能变化，因此在编译和链接时都使用 -pthread 选项而不是传统的 -lpthread 能够保持向后兼容，并提高命令行的一致性。目前gcc 4.5.2中已经没有了关于 -lpthread的介绍了。所以以后的多线程编译应该用-pthread，而不是-lpthread。

#### 一个简单的多线程程序

```c
#include "thread.h"

void Ta() { while (1) { printf("a"); } }
void Tb() { while (1) { printf("b"); } }

int main() {
  create(Ta);
  create(Tb);
}

```

利用 `thread.h` 就可以写出利用多处理器的程序！

*   操作系统会自动把线程放置在不同的处理器上

*   在后台运行，可以看到 CPU 使用率超过了 100%

***

**如何证明线程确实共享内存？**

*   [shm-test.c](http://jyywiki.cn/pages/OS/2022/demos/shm-test.c "shm-test.c")

**如何证明线程具有独立堆栈 (以及确定它们的范围)？**

*   [stack-probe.c](http://jyywiki.cn/pages/OS/2022/demos/stack-probe.c "stack-probe.c") (输出有点乱？我们还有 `sort`!)

    利用`_thread` 可以定义一些(thread local)线程局部的变量，这个局部变量会给每一个线程创建一个副本

***

更多的习题

*   创建线程使用的是哪个系统调用？clone()

*   能不能用 gdb 调试？

*   基本原则：有需求，就能做到 ([RTFM](https://sourceware.org/gdb/onlinedocs/gdb/Threads.html "RTFM"))

#### POSIX Threads

想进一步配置线程？

*   设置更大的线程栈

*   设置 detach 运行 (不在进程结束后被杀死，也不能 join)

*   ……

***

POSIX 为我们提供了线程库 (pthreads)

*   `man 7 pthreads`

*   练习：改写 thread.h，使得线程拥有更大的栈

    *   可以用 [stack-probe.c](http://jyywiki.cn/pages/OS/2022/demos/stack-probe.c "stack-probe.c") 验证

# 放弃

## 原子性

### 原子性的丧失

> “程序 (甚至是一条指令) 独占处理器执行” 的基本假设在现代多处理器系统上不再成立。

原子性：一段代码执行 (例如 `pay()`) 独占整个计算机系统

*   单处理器多线程

    *   线程在运行时可能被中断，切换到另一个线程执行

*   多处理器多线程

    *   线程根本就是并行执行的

***

(历史) 1960s，大家争先在共享内存上实现原子性 (互斥)

*   但几乎所有的实现都是错的，直到 [Dekker's Algorithm](https://en.wikipedia.org/wiki/Dekker's_algorithm "Dekker's Algorithm")，还只能保证两个线程的互斥

### 线程安全

`printf` 还能在多线程程序里调用吗？

`void thread1() { while (1) { printf("a"); } } void thread2() { while (1) { printf("b"); } }`&#x20;

我们都知道 printf 是有缓冲区的 (为什么？)

*   如果执行 `buf[pos++] = ch` (`pos` 共享) 不就 💥 了吗？

***

### 实现原子性

并发程序的复杂性从根本上来说对人类是"失控"，但从另一个角度，人类在另外一个纬度解决这个问题的工程办法：
&#x20;               做出合适的抽象，并且只写自己能够控制了的代码

互斥和原子性是本学期的重要主题

*   `lock(&lk)`

*   `unlock(&lk)`

    *   实现临界区 (critical section) 之间的绝对串行化

    *   程序的其他部分依然可以并行执行

***

99% 的并发问题都可以用一个队列解决

*   把大任务切分成可以并行的小任务

*   worker thread 去锁保护的队列里取任务

*   除去不可并行的部分，剩下的部分可以获得线性的加速

## 顺序

### 例子：求和 (再次出现)

分两个线程，计算

```c
#define N 100000000
long sum = 0;

void Tsum() { for (int i = 0; i < N; i++) sum++; }

int main() {
  create(Tsum);
  create(Tsum);
  join();
  printf("sum = %ld\n", sum);
}
```

我们好像忘记给 [sum.c](http://jyywiki.cn/pages/OS/2022/demos/sum.c "sum.c") 添加编译优化了？

*   `-O1`: 100000000 😱😱

*   `-O2`: 200000000 😱😱😱

### 顺序的丧失

> 编译器对内存访问 “eventually consistent” 的处理导致共享内存作为线程同步工具的失效。

刚才的例子

*   `-O1`: `R[eax] = sum; R[eax] += N; sum = R[eax]`

*   `-O2`: `sum += N;`

*   (你的编译器也许是不同的结果)

另一个例子

`while (!done); // would be optimized to if (!done) while (1);`

### 实现源码顺序翻译

在代码中插入 “优化不能穿越” 的 barrier

*   `asm volatile ("" ::: "memory");`

    *   Barrier 的含义是 “可以读写任何内存”

*   使用 `volatile` 变量

    *   保持 C 语义和汇编语义一致

```c
extern int volatile done;

while (!done) ;

```

***

## 可见性

### 例子

```c
int x = 0, y = 0;

void T1() {
  x = 1;
  asm volatile("" : : "memory"); // compiler barrier
  printf("y = %d\n", y);
}

void T2() {
  y = 1;
  asm volatile("" : : "memory"); // compiler barrier
  printf("x = %d\n", x);

}
```

问题：我们最终能看到哪些结果？

*   [mem-ordering.c](http://jyywiki.cn/pages/OS/2022/demos/mem-ordering.c "mem-ordering.c")

    *   输出不好读？`| head -n 1000000 | sort | uniq -c`

### 现代处理器：处理器也是 (动态) 编译器！

单个处理器把汇编代码 (用电路) “编译” 成更小的

ops

*   RF\[9] = load(RF\[7] + 400)

*   store(RF\[12], RF\[13])

*   RF\[3] = RF\[4] + RF\[5]

    *   每个

\*   uop 都有 Fetch, Issue, Execute, Commit 四个阶段

***

在任何时刻，处理器都维护一个

uop 的 “池子”

*   每一周期向池子补充尽可能多的

*   uop

    *   “多发射”

*   每一周期 (在不违反编译正确性的前提下) 执行尽可能多的

*   uop

    *   “乱序执行”、“按序提交”

*   这就是《计算机体系结构》 (剩下就是木桶效应，哪里短板补哪里)

### 多处理器间即时可见性的丧失

> 满足单处理器 eventual memory consistency 的执行，在多处理器上可能无法序列化！

当x ≠y时，对x ,y的内存读写可以交换顺序

*   它们甚至可以在同一个周期里完成 (只要 load/store unit 支持)

*   如果写发生 cache miss，可以让读 先执行

*   满足 “尽可能执行uop 的原则，最大化处理器性能

```6502&#x20;assembly
# <-----------+
movl $1, (x)   #   |
movl (y), %eax # --+` 
```

*   在多处理器上的表现

    *   两个处理器分别看到y=0，x=0

# 总结

## 总结

本次课回答的问题

*   **Q**: 如何理解多处理器系统？

***

Take-away message

*   多处理器编程：入门

    *   多处理器程序 = 状态机 (共享内存；非确定选择线程执行)

    *   thread.h = create + join

*   多处理器编程：放弃你对 “程序” 的旧理解

    *   不原子、能乱序、不立即可见

        *   来自于编译优化 (处理器也是编译器)

        *   [Ad hoc synchronization considered harmful](https://www.usenix.org/events/osdi10/tech/full_papers/Xiong.pdf "Ad hoc synchronization considered harmful") (OSDI'10)
