# CO

This is a casual project of my own to realize a coroutine library
in C++.

## usage

```c++
#include <iostream>
#include "CO.h"

using namespace std;
using namespace CO;

void foo() {
  cout << "enter foo" << endl;
  co_yield();
  cout << "resume foo" << endl;
}

int main() {
  cout << "start main" << endl;
  co_begin(foo);
  cout << "returned from foo" << endl;
  Sleep(5000);
  cout << "returned from sleep" << endl;
  Scheduler *scheduler = GetScheduler();
  scheduler->RunForever();
  // returns when all coroutines are finished
  return 0;
}
```