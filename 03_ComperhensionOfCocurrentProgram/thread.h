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
   * @brief 
   * 
   * @param thread 当一个新的线程调用成功之后，就会通过这个参数将线程的句柄返回给调用者，以便对这个线程进行管理。
   * @param attr 指定线程所拥有的属性，当不需要修改线程的默认属性时，给它传递NULL就行
   * @param start_routine 函数指针， 当你的程序调用了这个接口之后，就会产生一个线程，而这个线程的入口函数就是start_routine()。如果线程创建成功，这个接口会返回void*
   * @param arg start_routine()函数有一个参数，这个参数就是pthread_create的最后一个参数arg。
      start_routine()有一个返回值，这个返回值可以通过pthread_join()接口获得。
   * @return * int 
   *int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg); 
   */        
  pthread_create(&(tptr->thread), NULL, wrapper, tptr);
  ++tptr;
}

void join() {
  for (int i = 0; i < NTHREAD; i++) {
    struct thread *t = &tpool[i];
    if (t->status == T_LIVE) {
      /**
       * 线程的合并是一种主动回收线程资源的方案。当一个进程或线程调用了针对其它线程的pthread_join()接口，就是线程合并了。
       * 这个接口会阻塞调用进程或线程，直到被合并的线程结束为止。当被合并线程结束，pthread_join()接口就会回收这个线程的资源，并将这个线程的返回值返回给合并者。
       * @param thread 代表需要你等待的线程
       * @param value_ptr 是一个指针，指向线程的返回值
       */
      pthread_join(t->thread, NULL);
      t->status = T_DEAD;
    }
  }
}

__attribute__((destructor)) void cleanup() {
  join();
}
