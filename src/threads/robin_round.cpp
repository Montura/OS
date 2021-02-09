#include <iostream>
#include <queue>
#include <cstdint>

int current_thread_id = -1;
uint64_t m_timeslice = 0;
uint64_t m_time = 0;
std::queue<int> thread_pool;

/**
 * Функция будет вызвана перед каждым тестом, если вы
 * используете глобальные и/или статические переменные
 * не полагайтесь на то, что они заполнены 0 - в них
 * могут храниться значения оставшиеся от предыдущих
 * тестов.
 *
 * timeslice - квант времени, который нужно использовать.
 * Поток смещается с CPU, если пока он занимал CPU функция
 * timer_tick была вызвана timeslice раз.
 **/
void scheduler_setup(int timeslice) {
  current_thread_id = -1;
  ::m_timeslice = timeslice;
  m_time = 0;
  thread_pool = {};
}

/**
 * Функция вызывается, когда создается новый поток управления.
 * thread_id - идентификатор этого потока и гарантируется, что
 * никакие два потока не могут иметь одинаковый идентификатор.
 **/
void new_thread(int thread_id) {
  if (current_thread_id == -1) {
    current_thread_id = thread_id;
    m_time = 0;
  } else {
    thread_pool.push(thread_id);
  }
}

/**
 * Функция вызывается, когда поток, исполняющийся на CPU,
 * завершается. Завершится может только поток, который сейчас
 * исполняется, поэтому thread_id не передается. CPU должен
 * быть отдан другому потоку, если есть активный
 * (незаблокированный и незавершившийся) поток.
 **/
void exit_thread() {
  if (thread_pool.empty()) {
    current_thread_id = -1;
  } else {
    current_thread_id = thread_pool.front();
    thread_pool.pop();
    m_time = 0;
  }
}

/**
 * Функция вызывается, когда поток, исполняющийся на CPU,
 * блокируется. Заблокироваться может только поток, который
 * сейчас исполняется, поэтому thread_id не передается. CPU
 * должен быть отдан другому активному потоку, если таковой
 * имеется.
 **/
void block_thread() {
  if (thread_pool.empty()) {
    current_thread_id = -1;
  } else {
    current_thread_id = thread_pool.front();
    thread_pool.pop();
    m_time = 0;
  }
}

/**
 * Функция вызывается, когда один из заблокированных потоков
 * разблокируется. Гарантируется, что thread_id - идентификатор
 * ранее заблокированного потока.
 **/
void wake_thread(int thread_id) {
  if (current_thread_id == -1) {
    current_thread_id = thread_id;
    m_time = 0;
  } else {
    thread_pool.push(thread_id);
  }
}

/**
 * Ваш таймер. Вызывается каждый раз, когда проходит единица
 * времени.
 **/
void timer_tick() {
  ++m_time;
  if (m_time == m_timeslice) {
    if (!thread_pool.empty()) {
      thread_pool.push(current_thread_id);
      current_thread_id = thread_pool.front();
      thread_pool.pop();
    }
    m_time = 0;
  }
}

/**
 * Функция должна возвращать идентификатор потока, который в
 * данный момент занимает CPU, или -1 если такого потока нет.
 * Единственная ситуация, когда функция может вернуть -1, это
 * когда нет ни одного активного потока (все созданные потоки
 * либо уже завершены, либо заблокированы).
 **/
int current_thread() {
  return current_thread_id;
}

using namespace std;

static void test(int timeslice) {
  printf("Start test for RoundRobin algorithm\n");
  scheduler_setup(timeslice);

  new_thread(1);
  new_thread(2);
  new_thread(3);

  cout << "outside loop current thread: " << current_thread() << endl;

  int iter_count = 7;
  for (int i = 0; i < iter_count; i++) {
    timer_tick();
    cout << "current thread: " << current_thread() << endl;
  }
  new_thread(4);
  int block = current_thread();
  cout << "block thread: " << block << endl;
  block_thread();
  cout << "outside loop current thread: " << current_thread() << endl;
  for (int i = 0; i < iter_count; i++) {
    timer_tick();
    cout << "current thread: " << current_thread() << endl;
  }
  wake_thread(block);
  cout << "wake thread: " << block << endl;
  cout << "outside loop current thread: " << current_thread() << endl;
  for (int i = 0; i < iter_count; i++) {
    timer_tick();
    cout << "current thread: " << current_thread() << endl;
  }
  cout << "exit thread: " << current_thread() << endl;
  exit_thread();
  cout << "outside loop current thread: " << current_thread() << endl;
  for (int i = 0; i < iter_count; i++) {
    timer_tick();
    cout << "current thread: " << current_thread() << endl;
  }
  cout << "exit thread: " << current_thread() << endl;
  exit_thread();
  cout << "outside loop current thread: " << current_thread() << endl;
  for (int i = 0; i < iter_count; i++) {
    timer_tick();
    cout << "current thread: " << current_thread() << endl;
  }
  cout << "exit thread: " << current_thread() << endl;
  exit_thread();
  cout << "outside loop current thread: " << current_thread() << endl;
  for (int i = 0; i < iter_count; i++){
    timer_tick();
    cout << "current thread: " << current_thread() << endl;
  }
}

void testRoundRobin() {
  printf("---------- Start RoundRobin algorithm test ----------\n");
  cout << "TimeSlice is 4\n";
  test(4);
  cout << "\nTimeSlice is 10 \n";
  test(10);
  printf("---------- End RoundRobin algorithm test ----------\n");
}
