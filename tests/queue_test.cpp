#include "queue.h"
#include <atomic>
#include <chrono>
#include <functional>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mutex>
#include <thread>
#include <unordered_map>

TEST(QueueTest, BoundedQueue) {
  std::unordered_map<int, int> m;
  std::mutex map_mutex;
  bounded_queue<int> q{16};
  int N = 2000;
  int consumer_count = 5;
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
  std::thread producer1(produce);
  std::thread consumers[consumer_count];
  for (int i = 0; i < consumer_count; ++i) {
    consumers[i] = std::thread(std::bind(consume, i));
  }

  producer1.join();

  for (int i = 0; i < 40; ++i) {
    {
      std::lock_guard<std::mutex> guard(map_mutex);
      if (m.size() == N) {
        break;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }
  q.done();
  for (auto &c : consumers) {
    c.join();
  }
  ASSERT_EQ(m.size(), N);
}