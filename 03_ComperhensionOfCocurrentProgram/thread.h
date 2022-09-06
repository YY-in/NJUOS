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
  //结构体指针的定义
  *tptr = (struct thread) {
    .id = tptr - tpool + 1,
    .status = T_LIVE,
    .entry = fn,
  };
  /**
   *int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg); 
   1. 线程句柄 thread：当一个新的线程调用成功之后，就会通过这个参数将线程的句柄返回给调用者，以便对这个线程进行管理。
   2. 入口函数 start_routine()： 当你的程序调用了这个接口之后，就会产生一个线程，而这个线程的入口函数就是start_routine()。
      如果线程创建成功，这个接口会返回0。
   3. 入口函数参数 *arg : start_routine()函数有一个参数，这个参数就是pthread_create的最后一个参数arg。
      这种设计可以在线程创建之前就帮它准备好一些专有数据，最典型的用法就是使用C++编程时的this指针。
      start_routine()有一个返回值，这个返回值可以通过pthread_join()接口获得。
   4. 线程属性 attr： pthread_create()接口的第二个参数用于设置线程的属性。
      这个参数是可选的，当不需要修改线程的默认属性时，给它传递NULL就行。具体线程有那些属性，我们后面再做介绍。
   */
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
