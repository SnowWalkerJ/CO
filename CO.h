#ifndef CO_LIBRARY_H
#define CO_LIBRARY_H
#include <queue>
#include <list>
#define STACK_SIZE (128*1024)
namespace CO {
namespace detail {
using namespace std;

struct Registers {
  void *r12;
  void *r13;
  void *r14;
  void *r15;
  void *rip;
  void *rsp;
  void *rbx;
  void *rbp;
};

struct Stack {
  char stack_space[STACK_SIZE];
  void *const stack_addr;
  Stack() : stack_addr(reinterpret_cast<void *>(
                           (reinterpret_cast<unsigned long long>(stack_space) + STACK_SIZE - sizeof(void *))
                               & ~15ull)) {}
  Stack(const Stack &) = delete;
  Stack(Stack &&) = delete;
  const Stack &operator=(const Stack &) = delete;
  const Stack &operator=(Stack &&) = delete;
};

struct Context {
  Registers reg;
  Stack stack;
  Context(void (*function)()=nullptr);
  Context(const Context&) = delete;
  Context(Context&&) = delete;
  template<typename T>
  void Push(T value) {
    reg.rsp = static_cast<char *>(reg.rsp) - sizeof(T);
    *reinterpret_cast<T *>(reg.rsp) = value;
  }
};

struct Task {
  Task(Context *, unsigned long);
  bool operator<(const Task &other) const;
  Context *context;
  unsigned long expire_time;
};

class Scheduler {
 public:
  Scheduler();
  void AddTask(Context *context, unsigned long expire_time);
  Context * AddCoroutine(void (*)());
  Context * AddCoroutine();
  Context * CurrentContext();
  void RemoveContext(Context *context);
  void ActivateNextTask();
  void Checkout(Context *);
  void RunForever();
 private:
  Context *current_context_ = nullptr;
  list<Context> contexts_;
  priority_queue<Task> tasks_;
};

extern "C" void swap_context(void *, void *) asm("swap_context");
void co_end();
} // namespace detail
void co_begin(void (*func)());
void co_yield();
void Sleep(unsigned long ms);
using detail::Scheduler;
Scheduler *GetScheduler();

} // namespace CO
#endif //CO_LIBRARY_H
