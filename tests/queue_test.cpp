#include "queue.h"
#include <atomic>
#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mutex>
#include <thread>
#include <unordered_map>

TEST(QueueTest, BoundedQueue) {
  std::unordered_map<int, int> m;
  std::mutex map_mutex;
  bounded_queue<int> q{10};
  int N = 2000;
  auto produce = [&]() {
    for (int i = 0; i < N; ++i) {
      q.push(i);
    }
  };
  auto consume = [&](int cid) {
    int idx{};
    while (q.pop(idx)) {
      std::lock_guard<std::mutex> guard(map_mutex);
      ASSERT_TRUE(m.find(idx) == m.end());
      m[idx] = cid;
    }
  };
  std::thread producer1(std::bind(produce));
  std::thread consumer1(std::bind(consume, 1));
  std::thread consumer2(std::bind(consume, 2));

  producer1.join();
  for (int i = 0; i < 20; ++i) {
    std::lock_guard<std::mutex> guard(map_mutex);
    if (m.size() >= N) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  q.done();
  consumer1.join();
  consumer2.join();
  ASSERT_EQ(m.size(), N);
}