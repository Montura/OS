#include <atomic>
#include <cstdio>
#include <thread>
// Atomic RW register
//  'read' - atomically reads from the RW register
//  'write' - atomically writes to the RW register
//  'read' and 'write' operations are ordered!!!

typedef uint64_t ThID;

#ifdef _WINDOWS
  #include <windows.h>

  ThID threadId() {
    return GetCurrentThreadId();
  }
#else
  #include <pthread.h>

  ThID threadId() {
    return reinterpret_cast<uint64_t>(pthread_self());
  }
#endif

void dumpThreadId() {
  printf("Current thread id: %lld\n", threadId());
}

void testDumpingThId() {
  printf("---------- Start dumping thread id test ----------\n");
  std::thread th0(&dumpThreadId);
  std::thread th1(&dumpThreadId);

  th0.join();
  th1.join();

  printf("---------- End dumping thread id test ----------\n");
}

// Alternation
namespace Alternation {
// We have 2 threads: th_0 = 0, thId_1 = 1;
// + Has mutual exclusion - true
// - Has thread liveness - false
//   Ex:
//      1) Assume that th_1 never tries to lock a mutex.
//      2) So, after th_0 go to the infinite cycle in the first lock attempt:
//         threadId() == 0 and so (&lock->last == 0) is always true!

  struct Mutex {
    std::atomic<ThID> last;
  };

  void lock_init(Mutex *lock) {
    std::atomic_store(&lock->last, 0ULL);
  }

  void lock(struct Mutex *lock) {
    while (std::atomic_load(&lock->last) == threadId());
  }

  void unlock(struct Mutex *lock) {
    std::atomic_store(&lock->last, threadId());
  }
}

// IntentionFlags
namespace IntentionFlags {
// We have 2 threads: th_0 = 0, th_1 = 1;
// + Has mutual exclusion - true
// - Has thread liveness - false
//   Ex:
//      1) th_0 and th1 executes std::atomic_store(&lock->flag[me], 1);
//      2) They will hang forever in the lock cycle
//            while (std::atomic_load(&lock->flag[other]));

  struct Mutex {
    std::atomic<ThID> flag[2];
  };

  void lock_init(Mutex *lock) {
    std::atomic_store(&lock->flag[0], 0ULL);
    std::atomic_store(&lock->flag[1], 0ULL);
  }

  void lock(struct Mutex *lock) {
    const ThID me = threadId();
    const ThID other = 1 - threadId();

    std::atomic_store(&lock->flag[me], 1ULL);
    while (std::atomic_load(&lock->flag[other]));
  }

  void unlock(struct Mutex *lock) {
    const ThID me = threadId();
    std::atomic_store(&lock->flag[me], 0ULL);
  }
}

// Peterson Algorithm
namespace Peterson {
// We have 2 threads: th_0 = 0, th_1 = 1;
// + Has mutual exclusion - true
// + Has thread liveness - true

  struct Mutex {
    std::atomic<ThID> last;
    std::atomic<ThID> flag[2];
  };

  void lock_init(Mutex *lock) {
    std::atomic_store(&lock->last, 0ULL);
    std::atomic_store(&lock->flag[0], 0ULL);
    std::atomic_store(&lock->flag[1], 0ULL);
  }

  void lock(struct Mutex *lock) {
    const ThID me = threadId();
    const ThID other = 1 - threadId();

    // The order is important!
    std::atomic_store(&lock->flag[me], 1ULL);
    std::atomic_store(&lock->last, me);

    while (std::atomic_load(&lock->flag[other]) && std::atomic_load(&lock->last) == me);
  }

  void unlock(struct Mutex *lock) {
    const ThID me = threadId();
    std::atomic_store(&lock->flag[me], 0ULL);
  }

  //  Wrong order of instructions
  //    1.std::atomic_store(&lock->last, me);
  //    2. std::atomic_store(&lock->flag[me], 1);
  //  Ex:
  //  1. th0 and th1 pass atomic_store(&lock->last, me);
  //  2. th1 got to sleep, th0 is working
  //  3. th0: atomic_store(&lock->flag[0], 1); #### flags[0] = 1, flags[1] = 0
  //  4. th0: (atomic_load(&lock->flag[1]) && atomic_load(&lock->last) == 0) is false, so th0 goes to the critical section
  //  5. th1 is awake after th0 passes atomic_store(&lock->last, me)  !!!!
  //  6. lock->last is 0 !!!!
  //  7. th1: (atomic_load(&lock->flag[0]) && atomic_load(&lock->last) == 1) is false, so th1 also goes to the critical section
  //  8. th0 and th1 are inside the critical section!
}