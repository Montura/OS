#include <chrono>
#include <iostream>

namespace {
  using namespace std::chrono;

  void thread0() {
    const steady_clock::time_point& start = steady_clock::now();

    while (steady_clock::now() - start < seconds(10));
  }

  void thread1() {
    for (int i = 0; i < 20; ++i) {
      std::cout << "Hello world from thread1\n";
    }
  }
}

#ifdef _WINDOWS
#include <thread>
#include <windows.h>

typedef void* TheadHandle;

int testPriorityBoost() {
  std::cout << "Main started\n";

  std::thread th0(&thread0);
  TheadHandle h0 = th0.native_handle();
  SetThreadPriorityBoost(h0, true);
  bool setTh1 = SetThreadPriority(h0, THREAD_PRIORITY_HIGHEST);
  std::cout << "SetThreadPriority highest priority: " << setTh1 << "\n";

  std::thread th1(&thread1);
  TheadHandle h1 = th1.native_handle();
  SetThreadPriorityBoost(h1, true);
  bool setTh2 = SetThreadPriority(h1, THREAD_PRIORITY_LOWEST);
  std::cout << "SetThreadPriority highest priority: " << setTh2 << "\n";


  th0.join();
  th1.join();

  std::cout << "Main finished \n";
  return 0;
};
#endif