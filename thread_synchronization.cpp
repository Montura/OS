#include <atomic>
#include <cstdio>
#include <thread>
#include <array>
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
    lock->last.store(0ULL);
  }

  void lock(struct Mutex *lock) {
    while (lock->last.load() == threadId());
  }

  void unlock(struct Mutex *lock) {
    lock->last.store(threadId());
  }
}

// IntentionFlags
namespace IntentionFlags {
// We have 2 threads: th_0 = 0, th_1 = 1;
// + Has mutual exclusion - true
// - Has thread liveness - false
//   Ex:
//      1) th_0 and th1 executes lock->flag[me].store(1);
//      2) They will hang forever in the lock cycle
//            while (lock->flag[other].load()));

  struct Mutex {
    std::array<std::atomic<ThID>, 2> flag;
  };

  void lock_init(Mutex *lock) {
    lock->flag[0].store(0ULL);
    lock->flag[1].store(0ULL);
  }

  void lock(struct Mutex *lock) {
    const ThID me = threadId();
    const ThID other = 1 - threadId();

    lock->flag[me].store(1ULL);
    while (lock->flag[other].load());
  }

  void unlock(struct Mutex *lock) {
    const ThID me = threadId();
    lock->flag[me].store(0ULL);
  }
}

// Peterson Algorithm
namespace Peterson2Threads {
// We have 2 threads: th_0 = 0, th_1 = 1;
// + Has mutual exclusion - true
// + Has thread liveness - true

  struct Mutex {
    std::atomic<ThID> last;
    std::array<std::atomic<ThID>, 2> flag;
  };

  void lock_init(Mutex *lock) {
    lock->last.store( 0ULL);
    lock->flag[0].store(0ULL);
    lock->flag[1].store( 0ULL);
  }

  void lock(struct Mutex *lock) {
    const ThID me = threadId();
    const ThID other = 1 - threadId();

    // The order is important!
    lock->flag[me].store( 1ULL);
    lock->last.store(me);

    while (lock->flag[other].load() && (lock->last.load() == me));
  }

  void unlock(struct Mutex *lock) {
    const ThID me = threadId();
    lock->flag[me].store(0ULL);
  }

  //  Wrong order of instructions
  //    1.lock->last, me);
  //    2. lock->flag[me], 1);
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

namespace PetersonGreedy {
// The is a competition between N threads.
// Only 1 thread will win and get lock. Others will be waiting for lock.

// level N    , 1 thread - the winner     :                      |w|
// level N - 1, 1 thread - the winner :                        |b ? w|
// level N - 2, 1 thread - the winner :                 |b ? l|       |w ? z|
// -------------------------------------------------------------------------------------------------
// level k, N - 2^k threads           :     |a ? b|..... |c ? d| .... |i ? j| ..... |y ? z|
// -------------------------------------------------------------------------------------------------
// level 0, N threads                 :     |0 ? 1|     |1 ? 2| ..... |i ? j| .... |n - 1 ? n|

  constexpr int N = 10;

// Default implementation with O(N^2) atomics

// Step 1. Introduce new struct LockOne with N flags.
// LockOne is used in lock_one function to start a competition between many threads
  struct LockOne {
    std::atomic<ThID> last;
    std::array<std::atomic<ThID>, N> flags;
  };

  bool flags_clear(LockOne *lock) {
    const ThID me = threadId();

    for (int i = 0; i < N; ++i) {
      if (i != me && lock->flags[i].load()) {
        return false;
      }
    }

    return true;
  }

// "Filter function" guarantees mutual exclusion.
// If N threads call lock_one then N - 1 pass, and a single thread remains to wait for
  void lock_one(LockOne *lock) {
    const ThID me = threadId();

    lock->flags[me].store(1);
    lock->last.store(me);

    while (!flags_clear(lock) && (lock->last.load() == me));
  }

  void unlock_one(LockOne *lock) {
    const ThID me = threadId();

    lock->flags[me].store(0);
  }

  // Peterson algorithm with O(N^2) memory
  struct Mutex {
    std::array<LockOne, N -1> locks;
  };

  void lock(Mutex *mutex) {
    // N - 1 times we have to win to pass to the critical section while other threads loose and are waiting for the lock
    for (int level = 0; level < N - 1; ++level) {
      lock_one(&mutex->locks[level]);
    }
  }

  void unlock(Mutex *mutex) {
    // We have to unlock flags for all threads [0, N - 2]
    for (int level = 0; level < N - 1; ++level) {
      unlock_one(&mutex->locks[level]);
    }
  }
}

// Optimization by memory
// Above we used (N - 1) * N flags -> O(N^2) atomic registers
// Ex: thread[k] and level[i]
//    if (lock[i].flags[k] == 1) |=> For every {j, j < i} |=> lock[j].flags[k] = 1;
// So if the thread intended to get a lock on the level[i] then he has already the same intention on the previous level[j]
// We have following sequences [00000.....111111], when 1 is set on proper level
// This array can be replaced with LEVEL value!

// We can optimize it using only O(N) atomic registers to sync N threads

namespace OptimizedPeterson {
  constexpr int N = 10;

  struct Mutex {
    std::array<std::atomic<ThID>, N> levels; // the length of the 1's prefix|postfix (1111 ... 000 or 000....11111)
    std::array<std::atomic<ThID>, N - 1> last;
  };

  bool flags_clear(Mutex* lock, int level) {
    const ThID me = threadId();

    for (int i = 0; i < N; ++i) {
      if (i != me && (lock->levels[i].load() == level)) {
        return false;
      }
    }

    return true;
  }

  void lock(Mutex* lock) {
    const ThID me = threadId();

    for (int level = 0; level < N - 1; ++level) {
      lock->levels[me].store(level + 1);
      lock->last[level].store(me);

      while (!flags_clear(lock, level) && (lock->last[level].load() == me));
    }
  }

  void unlock(Mutex* lock) {
    const ThID me = threadId();
    lock->levels[me].store(0ULL);
  }
}