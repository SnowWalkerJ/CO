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
  return 0;
}