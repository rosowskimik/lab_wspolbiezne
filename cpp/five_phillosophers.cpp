#include <algorithm>
#include <array>
#include <chrono>
#include <format>
#include <iostream>
#include <latch>
#include <random>
#include <semaphore>
#include <thread>

void sleep_rand(unsigned int max_dur) {
  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_int_distribution<unsigned int> dist(0, max_dur);

  std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
}

enum { PHIL = 5 };

inline const size_t left(const size_t i) {
  return (i == 0) ? 4 : i - 1;
}

inline const size_t right(const size_t i) {
  return i;
}

#define BS std::binary_semaphore(1)
std::array<std::binary_semaphore, PHIL> forks = {BS, BS, BS, BS, BS};
std::array<std::jthread, PHIL> phils;

void philosopher(size_t i, std::latch* latch) {
  std::pair<size_t, size_t> minmax = std::minmax(left(i), right(i));

  const size_t l = minmax.first, r = minmax.second;
  bool first = true;

  for (;;) {
    std::cout << std::format("Phillosopher {} wants to eat\n", i);

    if (first) {
      first = !first;
      latch->arrive_and_wait();
    }
    forks[l].acquire();
    forks[r].acquire();

    std::cout << std::format(
        "Phillosopher {} starts eating (Forks {} and {})\n", i, l, r);

    sleep_rand(5000);

    std::cout << std::format("Phillosopher {} finished eating\n", i);

    forks[l].release();
    forks[r].release();

    std::cout << std::format("Phillosopher {} is thinking\n", i);

    sleep_rand(5000);
  }
}

int main() {
  std::latch l(PHIL);

  for (size_t i = 0; i < PHIL; ++i) {
    phils[i] = std::jthread(philosopher, i, &l);
  }
}
