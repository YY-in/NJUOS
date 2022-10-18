#include "thread.h"

#define N 100000000

long sum = 0;

void Tsum() {
  for (int i = 0; i < N; i++) {
    sum++;
  }
}

int main() {
  create(Tsum);
  create(Tsum);
  // 同步，等待某个条件起作用则继续运行
  // 此处等所有线程结束后继续执行，否则等待
  join();
  printf("sum = %ld\n", sum);
}
