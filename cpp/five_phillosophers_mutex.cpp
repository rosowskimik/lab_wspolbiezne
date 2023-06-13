#include <algorithm>
#include <array>
#include <chrono>
#include <format>
#include <iostream>
#include <latch>
#include <mutex>
#include <random>
#include <thread>

void sleep_rand(unsigned int max_dur) {
  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_int_distribution<unsigned int> dist(0, max_dur);

  std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
}

enum { PHIL = 5 };
#define LEFT(i) ((i == 0) ? 4 : i - 1)
#define RIGHT(i) (i)

std::array<std::mutex, PHIL> forks;
std::array<std::jthread, PHIL> phils;

void philosopher(size_t i, std::latch* latch) {
  const size_t l = LEFT(i), r = RIGHT(i);
  bool first = true;

  for (;;) {
    std::cout << std::format("Phillosopher {} wants to eat\n", i);

    {
      if (first) {
        first = !first;
        latch->arrive_and_wait();
      }
      std::scoped_lock lck(forks[l], forks[r]);

      std::cout << std::format(
          "Phillosopher {} starts eating (Forks {} and {})\n", i, l, r);

      sleep_rand(5000);

      std::cout << std::format("Phillosopher {} finished eating\n", i);
    }

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
