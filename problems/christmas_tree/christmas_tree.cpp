#include <algorithm>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <format>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <utility>

void rsleep(unsigned int max_dur) {
  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_int_distribution<unsigned int> dist(0, max_dur);

  std::this_thread::sleep_for(std::chrono::milliseconds(dist(gen)));
}

#ifndef FLOORS
#define FLOORS 20
#endif

#ifndef DECORS
#define DECORS 5
#endif

#ifndef ELFS
#define ELFS 2000
#endif

enum Direction {
  Down = -1,
  Up = 1,
};

struct Floor {
  std::mutex m;
  std::condition_variable cv;
  size_t on_floor;
};

template <size_t floor_cnt, size_t buf_size>
class Tree {
 private:
  std::array<Floor, floor_cnt> m_floors;

  struct {
    size_t size;
    std::mutex m;
    std::condition_variable full;
    std::condition_variable empty;
  } m_buffer;

 public:
  Tree() {
    // Start with buffer full
    m_buffer.size = buf_size;

    auto it = m_floors.begin();
    (it++)->on_floor = ELFS;
    std::for_each(it, m_floors.end(), [](auto& f) { f.on_floor = 0; });
  }

  void place() {
    {
      std::unique_lock lck(m_buffer.m);
      m_buffer.full.wait(lck, [&]() { return m_buffer.size < buf_size; });
      m_buffer.size += 1;
    }
    m_buffer.empty.notify_one();
  }

  void pickup() {
    {
      std::unique_lock lck(m_buffer.m);
      m_buffer.empty.wait(lck, [&]() { return m_buffer.size > 0; });
      m_buffer.size -= 1;
    }
    m_buffer.full.notify_one();
  }

  void move_floor(std::ptrdiff_t base, Direction dir) {
    size_t next = base + dir;
    Floor &curr_f = m_floors.at(base), &next_f = m_floors.at(next);

    {
      std::unique_lock lck_n(next_f.m);
      next_f.cv.wait(lck_n, [&]() {
        return (dir == Direction::Up && floor_cnt - 1 != next)
                   ? has_empty_space(next, 2)
                   : has_empty_space(next);
      });
      std::unique_lock lck_c(curr_f.m);

      ++next_f.on_floor;
      --curr_f.on_floor;
    }
    curr_f.cv.notify_all();
  }

  inline constexpr size_t on_floor(size_t floor) const {
    return m_floors[floor].on_floor;
  }
  inline constexpr size_t floor_count() const { return floor_cnt; }

 private:
  inline constexpr size_t empty_space(size_t target) const {
    return max_on_floor(target) - m_floors[target].on_floor;
  }

  inline constexpr bool has_empty_space(size_t target,
                                        size_t min_space = 1) const {
    if (target == 0) {
      return true;
    }

    return empty_space(target) >= min_space;
  }

  inline constexpr size_t max_on_floor(size_t floor) const {
    if (floor == 0) {
      return SIZE_MAX;
    }

    return m_floors.size() - floor;
  }
};

int main() {
  Tree<FLOORS, DECORS> tree;
  std::array<std::jthread, ELFS> elfs;

  // Elf threads
  std::generate(elfs.begin(), elfs.end(), [&, id = 0]() mutable {
    return std::jthread(
        [&](size_t id) {
          size_t my_floor = 0;

          auto move = [&, id](Direction dir) {
            tree.move_floor(my_floor, dir);

            std::cout << std::format(
                "Elf {:04} moved from floor {:02} to {:02}\n", id, my_floor,
                my_floor + dir);
            my_floor += dir;
            rsleep(2500);
          };

          for (;;) {
            // Pickup from the buffer
            tree.pickup();
            std::cout << std::format("Elf {:04} takes decor from the ground\n",
                                     id);

            // Climb to the top
            while (my_floor < tree.floor_count() - 1) {
              move(Direction::Up);
            }
            std::cout << std::format("Elf {:04} climbed to the top\n", id);

            // Move down
            while (my_floor > 0) {
              move(Direction::Down);
            }
            std::cout << std::format("Elf {:04} climbed down\n", id);
          }
        },
        id++);
  });

  // Santa thread
  for (;;) {
    rsleep(1000);
    tree.place();
    std::cerr << "Santa placed new decor\n";
  }

  return 0;
}
