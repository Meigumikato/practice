#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <concepts>
#include <coroutine>
#include <cstring>
#include <ctime>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <type_traits>

enum class EventType : int {
  TIMER,
  SOCKET,
};

class IoContext : public std::enable_shared_from_this<IoContext> {
 public:
  IoContext()
      : event_fd_(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
        // timer_fd_(timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK |TFD_CLOEXEC)),
        epoll_fd_(epoll_create(100)) {
    if (event_fd_ == -1) {
      spdlog::throw_spdlog_ex(strerror(errno));
    }

    if (epoll_fd_ == -1) {
      spdlog::throw_spdlog_ex(strerror(errno));
    }

    ::epoll_event eventfd_data;
    eventfd_data.events = EPOLLIN;
    eventfd_data.data.fd = event_fd_;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, event_fd_, &eventfd_data);
  }

  ~IoContext() {
    close(event_fd_);
    close(epoll_fd_);
  }

  auto Get() -> std::shared_ptr<IoContext> { return this->shared_from_this(); }

  void Run() {
    for (;;) {
      Launch();
    }
  }

  void RunOnce() { Launch(); }

  void Launch() {
    std::vector<::epoll_event> events(100);
    spdlog::info("event loop");
    int cnt = ::epoll_wait(epoll_fd_, events.data(), 100, -1);

    if (cnt == -1) throw;

    std::for_each_n(events.begin(), cnt, [&](::epoll_event& event) {
      auto fd = event.data.fd;

      if (fd == event_fd_) {
        ::eventfd_t x;
        ::eventfd_read(event_fd_, &x);
        ProcessCoroutine();

      } else {
        if (timer_map_.contains(fd)) {
          auto co_handle = timer_map_[fd];
          if (co_handle && !co_handle.done()) {
            co_handle.resume();
          }

          if (co_handle.done()) {
            co_handle.destroy();
            timer_map_.erase(fd);
            ::itimerspec tmr{};
            ::timerfd_settime(fd, 0, &tmr, &tmr);
          } else {
            ::itimerspec tmr{};
            ::timerfd_settime(fd, 0, &tmr, &tmr);
          }
        }
      }
    });
  }

  template <typename Awaitable>
  void Execute(Awaitable&& awaitable) {
    std::scoped_lock lock{mutex_};
    co_handles_.push(awaitable.Detach());
    // co_handles_.push(h);
    ::eventfd_write(event_fd_, 1);
  }

  void Register(::epoll_event* edata, std::coroutine_handle<> h) {
    // std::scoped_lock lock{mutex_};
    auto fd = edata->data.fd;
    timer_map_.insert(std::make_pair(fd, h));
    int ret = ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, edata->data.fd, edata);
    assert(ret != -1);
  }

 private:
  void ProcessCoroutine() {
    std::lock_guard lock{mutex_};
    while (!co_handles_.empty()) {
      auto& task = co_handles_.front();
      task.resume();
      if (task.done()) {
      }
      co_handles_.pop();
    }
  }

  int event_fd_;
  int epoll_fd_;

  std::mutex mutex_;
  std::queue<std::coroutine_handle<>> co_handles_;
  std::unordered_map<int, std::coroutine_handle<>> timer_map_;
};

class Timer {
 public:
  Timer(IoContext& ctx) : timer_fd_(::timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC | TFD_NONBLOCK)), io_context_(&ctx) {}

  void ExpireAfter(int x) {
    ::itimerspec utmr{};
    utmr.it_value.tv_sec = x;
    timerfd_settime(timer_fd_, 0, &utmr, nullptr);
  }

  void ExpireAt(int x) {}

  int GetNativeHandle() { return timer_fd_; }

  auto AsyncWait() {
    struct Awaiter {
      auto await_ready() { return false; }
      auto await_suspend(std::coroutine_handle<> h) {
        ::epoll_event edata;
        edata.data.fd = timer->GetNativeHandle();
        edata.events = EPOLLIN;
        ctx->Register(&edata, h);
      }

      auto await_resume() {}
      Timer* timer;
      IoContext* ctx;
    };

    return Awaiter{this, io_context_};
  }

 private:
  int timer_fd_;
  IoContext* io_context_;
};
