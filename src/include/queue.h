#include <condition_variable>
#include <iomanip>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>

template <typename T> class bounded_queue {
public:
  bounded_queue(std::size_t max_size) : m_max_size{max_size} {
    if (!m_max_size)
      throw std::invalid_argument("bad queue max-size! must be non-zero!");
  }

  ~bounded_queue() { done(); };

  bool push(const T &item) {
    {
      std::unique_lock<std::mutex> guard(m_queue_lock);
      m_condition_push.wait(
          guard, [&]() { return m_queue.size() < m_max_size || m_done; });
      if (m_done)
        return false;
      m_queue.push(item);
    }
    m_condition_pop.notify_one();
    return true;
  }

  bool push(T &&item) {
    {
      std::unique_lock<std::mutex> guard(m_queue_lock);
      m_condition_push.wait(
          guard, [&]() { return m_queue.size() < m_max_size || m_done; });
      if (m_done)
        return false;
      m_queue.push(std::move(item));
    }
    m_condition_pop.notify_one();
    return true;
  }

  bool pop(T &item) {
    {
      std::unique_lock<std::mutex> guard(m_queue_lock);
      m_condition_pop.wait(guard, [&]() { return !m_queue.empty() || m_done; });
      if (m_done == true)
        return false;
      item = std::move(m_queue.front());
      m_queue.pop();
    }
    m_condition_push.notify_one();
    return true;
  }

  std::size_t size() const {
    std::unique_lock<std::mutex> guard(m_queue_lock);
    return m_queue.size();
  }

  bool empty() const {
    std::unique_lock<std::mutex> guard(m_queue_lock);
    return m_queue.empty();
  }

  void done() {
    {
      std::unique_lock<std::mutex> guard(m_queue_lock);
      m_done = true;
    }
    m_condition_push.notify_all();
    m_condition_pop.notify_all();
  }

private:
  using queue_t = std::queue<T>;
  queue_t m_queue;
  mutable std::mutex m_queue_lock;
  std::condition_variable m_condition_push;
  std::condition_variable m_condition_pop;
  std::size_t m_max_size;
  bool m_done = false;
};