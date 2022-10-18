#include "thread.h"
#include "thread-sync.h"

int n, count = 0;
mutex_t lk = MUTEX_INIT();

// 左括号-出题者
void Tproduce() {
  while (1) {
// retry:
    //先🔒
    mutex_lock(&lk);
    //等待条件：如果我的包满了
    if (count != n) {
      //🔓
      mutex_unlock_and_sleep(&lk);
      mutex_lock(&lk);
    }
    count++;
    printf("(");
    wakeup();
    mutex_unlock(&lk);
  }
}

void Tconsume() {
  while (1) {
// retry: //不希望自旋,则使用条件变量
    //同步的共性操作
    // 先上一把🔒
    mutex_lock(&lk);
    // 判断条件是否成立
    /**
     * @brief
     * 此处存在自旋，我们希望将自旋变成顺延
      if (!(count > 0)) {
      mutex_unlock(&lk);
      goto retry;
    }
     */
    if (!(count > 0)) {
      //条件不成立则将这把🔒释放掉,保持原子性
      mutex_unlock_and_sleep(&lk);
      mutex_lock(&lk);
    }
    // 条件成立继续执行
    count--;
    printf(")");
    wakeup();
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
