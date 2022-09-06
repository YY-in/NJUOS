#include "thread.h"
//利用优化的方式编译
#define N 100000000

long sum = 0;

void Tsum() {
  for (int i = 0; i < N; i++) {
    /**
     * 内联汇编：指令模版
     * %0 %1 等表示操作数，在模版中称之为占位符
     * m对应%0，表示使用内存地址
     * @brief lock 可以理解为全世界都停下来了，然后单独做某件事
     */
    asm volatile("lock addq $1, %0": "+m"(sum));
  }
}

int main() {
  create(Tsum);
  create(Tsum);
  join();
  printf("sum = %ld\n", sum);
}
