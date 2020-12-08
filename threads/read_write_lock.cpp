#include <atomic>

namespace {
  struct RWLock {
    // Queue for threads
    std::atomic_uint64_t ticket;
    // Readers and writers
    std::atomic_uint64_t read;
    std::atomic_uint64_t write;
  };

  void read_lock(RWLock* lock) {
    uint64_t const ticket = lock->ticket.fetch_add(1);

    while (lock->read.load() != ticket);
    lock->read.store(ticket + 1);
  }

  void read_unlock(RWLock* lock) {
    lock->write.fetch_add(1);
  }

  void write_lock(RWLock* lock) {
    uint64_t const ticket = lock->ticket.fetch_add(1);

    while (lock->write.load() != ticket);
  }

  void write_unlock(RWLock* lock) {
    lock->read.fetch_add(1);
    lock->write.fetch_add(1);

  }
}