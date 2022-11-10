
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
  StaticThreadPool(int num = 4) : stopped_(false) {
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
      auto await_ready() {}
      auto await_suspend(std::coroutine_handle<> h) {
        spdlog::info("Schedule Suspend {}", fmt::ptr(std::this_thread::get_id()));
        pool->Post(h);
      }
      auto await_resume() {
        spdlog::info("Schedule Resume {}", fmt::ptr(std::this_thread::get_id()));
      }

      StaticThreadPool* pool;
    };

    return Awaiter{};
  }

 private:

  void Run() {
    while (1) {
      std::unique_lock lock{mutex_};
      while (!task_queue_.empty()) {
        cv_.wait(lock, [this] { return stopped_ || !task_queue_.empty(); });
      }

      auto task = task_queue_.front();
      task_queue_.pop_front();
      task.resume();

      if (stopped_) break;
    }
  }

  void Post(std::coroutine_handle<> h) {
    std::unique_lock lock{mutex_};
    task_queue_.push_back(h);
  }

  bool stopped_;

  std::mutex mutex_;
  std::condition_variable cv_;

  std::deque<std::coroutine_handle<>> task_queue_;
  std::vector<std::thread> threads_;
};
