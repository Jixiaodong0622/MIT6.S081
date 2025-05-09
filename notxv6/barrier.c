#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

static int nthread = 1;
static int round = 0;

struct barrier
{
  pthread_mutex_t barrier_mutex; // 互斥锁
  pthread_cond_t barrier_cond;   // 条件变量
  int nthread;                   // 当前到达这个屏障的线程数量
  int round;                     // Barrier round
} bstate;

// 初始化barrier结构体
static void
barrier_init(void)
{
  assert(pthread_mutex_init(&bstate.barrier_mutex, NULL) == 0); // 初始化互斥锁
  assert(pthread_cond_init(&bstate.barrier_cond, NULL) == 0);   // 初始化条件变量
  bstate.nthread = 0;
}

static void
barrier()
{
  // YOUR CODE HERE
  //
  // Block until all threads have called barrier() and
  // then increment bstate.round.
  //
  // 需要互斥访问的是bstate.nthread，如果bstate.nthread >= nthread，就要唤醒睡眠的线程，并清除bstate的线程数
  // 现在需要考虑的是在哪上锁
  // 将设当线程P进入屏障后，此时nthread+1，但是未达到全局变量nthread，那么我就得将该进程睡眠。
  // 但是，假设有这样一种情况，线程P正要睡眠的同时，线程Q来了，并且此时已经达到全局变量nthread，那我们就应该唤醒所有睡眠的线程。唤醒完之后，线程P才进入睡眠，按理说，我们在执行唤醒操作的时候，也应该将线程P唤醒。
  // 所有上完锁，我们得保证如果没有达到全局变量nthread，线程得进去睡眠状态
  pthread_mutex_lock(&bstate.barrier_mutex);
  if (++bstate.nthread < nthread)
  {
    pthread_cond_wait(&bstate.barrier_cond, &bstate.barrier_mutex);
  }
  else
  {
    bstate.nthread = 0;
    bstate.round++;
    pthread_cond_broadcast(&bstate.barrier_cond);
  }
  pthread_mutex_unlock(&bstate.barrier_mutex);
}

static void *
thread(void *xa)
{
  long n = (long)xa;
  long delay;
  int i;

  for (i = 0; i < 20000; i++)
  {
    int t = bstate.round;
    assert(i == t);
    barrier();
    usleep(random() % 100);
  }

  return 0;
}

int main(int argc, char *argv[])
{
  pthread_t *tha;
  void *value;
  long i;
  double t1, t0;

  if (argc < 2)
  {
    fprintf(stderr, "%s: %s nthread\n", argv[0], argv[0]);
    exit(-1);
  }
  nthread = atoi(argv[1]);
  tha = malloc(sizeof(pthread_t) * nthread);
  srandom(0);

  barrier_init();

  for (i = 0; i < nthread; i++)
  {
    assert(pthread_create(&tha[i], NULL, thread, (void *)i) == 0);
  }
  for (i = 0; i < nthread; i++)
  {
    assert(pthread_join(tha[i], &value) == 0);
  }
  printf("OK; passed\n");
}
