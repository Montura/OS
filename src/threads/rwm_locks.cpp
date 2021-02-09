#include <atomic>

namespace RMWLock {
  constexpr int LOCKED = 1;
  constexpr int UNLOCKED = 0;

  struct Mutex {
    std::atomic<uint64_t> locked;
  };

  void lock(Mutex * lock) {
    while (lock->locked.exchange(LOCKED) != UNLOCKED);
  }

  void unlock(Mutex * lock) {
    lock->locked.store(UNLOCKED);
  }
}

namespace TicketLock {
  // Non-explicit thread queue is implemented using tickets

  struct Mutex {
    std::atomic<uint64_t> next;
    std::atomic<uint64_t> ticket;
  };

  void lock(Mutex * lock) {
    auto ticket = lock->ticket.fetch_add(1);
    while (lock->next.load() != ticket);
  }

  void unlock(Mutex * lock) {
    lock->next.fetch_add(1);
  }
}