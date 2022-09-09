#include "thread.h"
#include "thread-sync.h"

int n, count = 0;
mutex_t lk = MUTEX_INIT();

// 左括号-出题者
void Tproduce() {
  while (1) {
retry:
    //先🔒
    mutex_lock(&lk);
    //如果我的包满了
    if (count == n) {
      mutex_unlock(&lk);
      goto retry;
    }
    count++;
    printf("(");
    mutex_unlock(&lk);
  }
}

void Tconsume() {
  while (1) {
retry:
    mutex_lock(&lk);
    if (count == 0) {
      mutex_unlock(&lk);
      goto retry;
    }
    count--;
    printf(")");
    mutex_unlock(&lk);
  }
}

int main(int argc, char *argv[]) {
  assert(argc == 2);
  n = atoi(argv[1]);
  setbuf(stdout, NULL);
  for (int i = 0; i < 8; i++) {
    create(Tproduce);
    create(Tconsume);
  }
}
