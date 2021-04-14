#include "CO.h"
#include <ctime>
#include <unistd.h>
#ifndef _unlikely
#define _unlikely(x) __builtin_expect((x), 0)
#endif

namespace CO {
namespace detail {
using namespace std;
Scheduler *scheduler = nullptr;
constexpr unsigned long SLICE_DURATION = 1000000ul;
asm(R"(
swap_context:
  mov 0x00(%rsp), %rdx
  lea 0x08(%rsp), %rcx
  mov %r12, 0x00(%rdi)
  mov %r13, 0x08(%rdi)
  mov %r14, 0x10(%rdi)
  mov %r15, 0x18(%rdi)
  mov %rdx, 0x20(%rdi)
  mov %rcx, 0x28(%rdi)
  mov %rbx, 0x30(%rdi)
  mov %rbp, 0x38(%rdi)
  mov 0x00(%rsi), %r12
  mov 0x08(%rsi), %r13
  mov 0x10(%rsi), %r14
  mov 0x18(%rsi), %r15
  mov 0x20(%rsi), %rax
  mov 0x28(%rsi), %rcx
  mov 0x30(%rsi), %rbx
  mov 0x38(%rsi), %rbp
  mov %rcx, %rsp
  jmpq *%rax
)");

unsigned long GetTime() {
  timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_nsec + ts.tv_sec * 1000000000ul;
}

void sleep(unsigned long ms) {
  usleep(ms);
}

Context::Context(void (*function)()) {
  reg.rip = reinterpret_cast<void *>(function);
  reg.rbp = reg.rsp = stack.stack_addr;
  Push(&co_end);
}

bool Task::operator<(const Task &other) const {
  return expire_time > other.expire_time;
}
Task::Task(Context *context, unsigned long expire_time) : context(context), expire_time(expire_time) {
}

Scheduler::Scheduler() {
  AddCoroutine(); // the main coroutine;
}
Context * Scheduler::AddCoroutine() {
  contexts_.emplace_back();
  Context *context = &contexts_.back();
  current_context_ = context;
  return context;
}
Context * Scheduler::AddCoroutine(void (*function)()) {
  contexts_.emplace_back(function);
  Context *context = &contexts_.back();
  return context;
}
Context * Scheduler::CurrentContext() {
  return current_context_;
}
void Scheduler::ActivateNextTask() {
  unsigned long now = GetTime();
  Task task = tasks_.top();
  tasks_.pop();
  if (task.expire_time > now) {
    sleep((task.expire_time - now) / 1000000);
  }
  Checkout(task.context);
}
void Scheduler::AddTask(Context *context, unsigned long expire_time) {
  tasks_.emplace(context, expire_time);
}
void Scheduler::Checkout(Context *context) {
  Context *ctx = current_context_;
  current_context_ = context;
  swap_context(ctx, current_context_);
}
void Scheduler::RemoveContext(Context *context) {
  for (auto iter = contexts_.begin(); iter != contexts_.end(); ++iter) {
    if (&*iter == context) {
      contexts_.erase(iter);
      break;
    }
  }
}
void Scheduler::RunForever() {
  while (!tasks_.empty()) {
    ActivateNextTask();
  }
}

void co_end() {
  using namespace detail;
  Scheduler *scheduler = GetScheduler();
  Context *context = scheduler->CurrentContext();
  scheduler->RemoveContext(context);
  scheduler->ActivateNextTask();
}

} // namespace detail

Scheduler *GetScheduler() {
  using namespace detail;
  if (_unlikely(scheduler == nullptr)) {
    scheduler = new Scheduler();
  }
  return scheduler;
}

void co_yield() {
  using namespace detail;
  Scheduler *scheduler = GetScheduler();
  scheduler->AddTask(scheduler->CurrentContext(), GetTime() + SLICE_DURATION);
  scheduler->ActivateNextTask();
}
void co_begin(void (*func)()) {
  using namespace detail;
  Scheduler *scheduler = GetScheduler();
  Context *co = scheduler->AddCoroutine(func);
  scheduler->AddTask(co, GetTime() + SLICE_DURATION);
  co_yield();
}

void Sleep(unsigned long ms) {
  using namespace detail;
  Scheduler *scheduler = GetScheduler();
  unsigned long now = GetTime();
  scheduler->AddTask(scheduler->CurrentContext(), now + ms * 1000000000ul);
  scheduler->ActivateNextTask();
}
} // namespace CO
