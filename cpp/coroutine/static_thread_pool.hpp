
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <deque>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <coroutine>
#include <condition_variable>


class StaticThreadPool { 
 public:
  StaticThreadPool(int num = std::thread::hardware_concurrency()) : stopped_(false) {
    for (int i = 0; i < num; ++i) {
      threads_.push_back(std::thread(&StaticThreadPool::Run, this));
    }
  }

  ~StaticThreadPool() {
    stopped_ = true;

    for (auto& thread : threads_) {
      thread.join();
    }
  }

  auto Schedule() {
    struct Awaiter {
      auto await_ready() { return false; }
      auto await_suspend(std::coroutine_handle<> h) {
        pool->Post(h);
      }
      auto await_resume() {
      }

      StaticThreadPool* pool;
    };

    return Awaiter{this};
  }

 private:

  void Run() {
    while (!stopped_) {
      std::unique_lock lock{mutex_};
      while (!task_queue_.empty()) {
        cv_.wait(lock, [this] { return stopped_ || !task_queue_.empty(); });
      }
      auto task = task_queue_.front();
      task_queue_.pop_front();

      task.resume();
    }
  }

  void Post(std::coroutine_handle<> h) {
    std::unique_lock lock{mutex_};
    task_queue_.push_back(h);
    cv_.notify_all();
  }

  bool stopped_;

  std::mutex mutex_;
  std::condition_variable cv_;

  std::deque<std::coroutine_handle<>> task_queue_;
  std::vector<std::thread> threads_;
};
