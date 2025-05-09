#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/* Possible states of a thread: */
// 线程的三种状态
#define FREE 0x0
#define RUNNING 0x1
#define RUNNABLE 0x2

// 每个线程最大的栈空间
#define STACK_SIZE 8192
// 最多有4个线程
#define MAX_THREAD 4

// 线程context内容，主要是各种寄存器
struct context
{
  uint64 ra;
  uint64 sp;
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

// 线程结构体，主要记录当前线程的状态跟线程的栈
struct thread
{
  char stack[STACK_SIZE]; /* the thread's stack */
  int state;              /* FREE, RUNNING, RUNNABLE */
  struct context context; // 线程的上下文属性
};
struct thread all_thread[MAX_THREAD]; // 线程列表，也可说是线程执行列表
struct thread *current_thread;
extern void thread_switch(struct context *old, struct context *new);

// 初始化线程
// 选择当前线程列表中第一个线程，并将其状态设置成RUNNING
void thread_init(void)
{
  // main() is thread 0, which will make the first invocation to
  // thread_schedule().  it needs a stack so that the first thread_switch() can
  // save thread 0's state.  thread_schedule() won't run the main thread ever
  // again, because its state is set to RUNNING, and thread_schedule() selects
  // a RUNNABLE thread.
  current_thread = &all_thread[0];
  current_thread->state = RUNNING;
}

// 线程调度，主要是寻找下一个状态为RUNNABLE的线程
void thread_schedule(void)
{
  // 初始化线程结构体
  struct thread *t, *next_thread;

  /* Find another runnable thread. */
  next_thread = 0;
  // t为当前正在执行的线程的下一个线程
  t = current_thread + 1;
  for (int i = 0; i < MAX_THREAD; i++)
  {
    // 如果t大于线程列表中最后一个线程，那么t就被赋值为第一个线程，有点像先来先服务的线程调度算法
    if (t >= all_thread + MAX_THREAD)
      t = all_thread;
    // 如果当前t的线程状态为RUNNABLE，那么线程调度的下一个线程就是t，将t赋值给next_thread
    if (t->state == RUNNABLE)
    {
      next_thread = t;
      break;
    }
    t = t + 1;
  }
  // 如果next_thread=0，说明没有找到一个状态为RUNNABLE的线程
  if (next_thread == 0)
  {
    printf("thread_schedule: no runnable threads\n");
    exit(-1);
  }

  // 如果当前线程与下一个被调度的线程不一样，则进行线程调度
  if (current_thread != next_thread)
  { /* switch threads?  */
    next_thread->state = RUNNING;
    t = current_thread;
    current_thread = next_thread;
    /* YOUR CODE HERE
     * Invoke thread_switch to switch from t to next_thread:
     * thread_switch(??, ??);
     */
    // 切换上下文
    thread_switch(&t->context, &next_thread->context);
  }
  else
    next_thread = 0;
}

// 创建线程，找到线程列表中状态为FREE的线程，并将其状态置为RUNNABLE
void thread_create(void (*func)())
{
  struct thread *t;

  for (t = all_thread; t < all_thread + MAX_THREAD; t++)
  {
    if (t->state == FREE)
      break;
  }
  t->state = RUNNABLE;
  // YOUR CODE HERE
  // 线程调度后续执行的应该是func()中的内容，所有将返回地址设置成func()
  t->context.ra = (uint64)func;
  // 还需要设置当前线程的栈指针
  t->context.sp = (uint64)&t->stack + (STACK_SIZE - 1);
}

void thread_yield(void)
{
  current_thread->state = RUNNABLE;
  thread_schedule();
}

volatile int a_started, b_started, c_started;
volatile int a_n, b_n, c_n;

void thread_a(void)
{
  int i;
  printf("thread_a started\n");
  a_started = 1;
  while (b_started == 0 || c_started == 0)
    thread_yield();

  for (i = 0; i < 100; i++)
  {
    printf("thread_a %d\n", i);
    a_n += 1;
    thread_yield();
  }
  printf("thread_a: exit after %d\n", a_n);

  current_thread->state = FREE;
  thread_schedule();
}

void thread_b(void)
{
  int i;
  printf("thread_b started\n");
  b_started = 1;
  while (a_started == 0 || c_started == 0)
    thread_yield();

  for (i = 0; i < 100; i++)
  {
    printf("thread_b %d\n", i);
    b_n += 1;
    thread_yield();
  }
  printf("thread_b: exit after %d\n", b_n);

  current_thread->state = FREE;
  thread_schedule();
}

void thread_c(void)
{
  int i;
  printf("thread_c started\n");
  c_started = 1;
  while (a_started == 0 || b_started == 0)
    thread_yield();

  for (i = 0; i < 100; i++)
  {
    printf("thread_c %d\n", i);
    c_n += 1;
    thread_yield();
  }
  printf("thread_c: exit after %d\n", c_n);

  current_thread->state = FREE;
  thread_schedule();
}

int main(int argc, char *argv[])
{
  a_started = b_started = c_started = 0;
  a_n = b_n = c_n = 0;
  thread_init();
  thread_create(thread_a);
  thread_create(thread_b);
  thread_create(thread_c);
  thread_schedule();
  exit(0);
}
