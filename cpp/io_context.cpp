#include <algorithm>
#include <cassert>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>

#include <cerrno>
#include <cstring>

#include <queue>
#include <mutex>
#include <concepts>
#include <coroutine>
#include <functional>
#include <type_traits>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include "task.hpp"


struct EventCallback {

  using CallBack = std::function<void()>;

  CallBack cb;
};

class IoContext {
 public:

  IoContext() : 
    event_fd_(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)), 
    timer_fd_(timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK |TFD_CLOEXEC)),
    epoll_fd_(epoll_create(100)) {

    if (event_fd_ == -1) {
      spdlog::throw_spdlog_ex(strerror(errno));
    }

    if (timer_fd_ == -1) {
      spdlog::throw_spdlog_ex(strerror(errno));
    }

    if (epoll_fd_ == -1) {
      spdlog::throw_spdlog_ex(strerror(errno));
    }

    ::epoll_event eventfd_data;
    eventfd_data.events = EPOLLIN;
    eventfd_data.data.fd = event_fd_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event_fd_, &eventfd_data);

    ::epoll_event timerfd_data;
    timerfd_data.events = EPOLLIN;
    timerfd_data.data.fd = timer_fd_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, timer_fd_, &timerfd_data);
  }

  ~IoContext() {
    close(event_fd_);
    close(timer_fd_);
    close(epoll_fd_);
  }

  void Run() {
    std::vector<::epoll_event> events(100);
    while (1) {
      spdlog::info("event loop");
      int cnt = ::epoll_wait(epoll_fd_, events.data(), 100, -1);

      if (cnt == -1) throw;

      std::for_each_n(events.begin(), cnt, [&] (::epoll_event& event) {
        if (event.data.fd == event_fd_) {
          ::eventfd_t x;
          ::eventfd_read(event_fd_, &x);
          spdlog::info("events read res val={}", x);
          ProcessEventTask();
          ProcessCoroutine();
        }

        if (event.data.fd == timer_fd_) {
          ::itimerspec utmr{}, otmr{};
          ::timerfd_settime(timer_fd_, 0, &otmr, &otmr);
          ProcessTimerTask();
        }

      });
    }
  }

  void Submit(std::function<void()>&& task) {
    std::lock_guard lock{mutex_};
    event_task_queue_.push(std::move(task));
    ::eventfd_write(event_fd_, 1);
  }

  void Spawn(std::coroutine_handle<> h) {
    co_handles_.push(h);
    ::eventfd_write(event_fd_, 1);
  }
 
  void StartAfter(std::function<void()>&& task, int s) {
    ::itimerspec utmr{};

    utmr.it_value.tv_sec = s;

    timerfd_settime(timer_fd_, 0, &utmr, nullptr);

    timer_task_queue_.push(std::move(task));
  }

  struct ScheduleOperation {
    bool await_ready() { return false; };
    void await_suspend(std::coroutine_handle<> h) {
      await = h;
      ctx->Submit([&] {
        await.resume();
      });
    }
    void await_resume() {
      spdlog::info("");
    }

    IoContext* ctx;
    std::coroutine_handle<> await;
  };


  ScheduleOperation Schedule() {
    return ScheduleOperation{this};
  }
  

  void ProcessEventTask() {
    while (!event_task_queue_.empty()) {
      auto& task = event_task_queue_.front();
      task();
      event_task_queue_.pop();
    }
  }


  void ProcessTimerTask() {
    while (!timer_task_queue_.empty()) {
      auto& task = timer_task_queue_.front();
      task();
      timer_task_queue_.pop();
    }
  }

  void ProcessCoroutine() {
    while (!co_handles_.empty()) {
      auto& task = co_handles_.front();
      task.resume();
      co_handles_.pop();
    }
  }


 private:

  int event_fd_;
  int timer_fd_;
  int epoll_fd_;

  std::mutex mutex_;

  std::queue<std::coroutine_handle<>> co_handles_;
  std::queue<std::function<void()>> event_task_queue_;
  std::queue<std::function<void()>> timer_task_queue_;

};


Task<int> Sum(int a, int b) {
  co_return a + b;
}

Task<int> Example(int a, int b, int c, int d) {
  int temp1 = co_await Sum(a, b);
  int temp2 = co_await Sum(c, d);

  int sum = co_await Sum(temp1, temp2);
  spdlog::info("coroutine Example execute");
  co_return sum;
}

void co_spawn(IoContext* ctx, Task<int>&& task) {
  ctx->Spawn(task.Detach());
}


int main() {

  IoContext ctx;

  ctx.Submit([]{
    spdlog::info("execute");
  });

  co_spawn(&ctx, Example(1, 2, 3, 4));

  ctx.Run();

  return 0;
}
