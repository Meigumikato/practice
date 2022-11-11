#include <mutex>
#include <atomic>
#include <condition_variable>
#include <concepts>



class ManualResetEvent {
 public:

  void Set() {
    std::unique_lock lock{mutex_};
    value_ = true;
    cv_.notify_one();
  }


  void Reset() {
    std::unique_lock lock{mutex_};
    value_ = false;
  }


  void Wait() {
    std::unique_lock lock{mutex_};
    while (!value_) {
      cv_.wait(lock, [this]{ return value_; });
      // cv_.wait(lock);
    }
  }

 private:
  bool value_{false};
  std::mutex mutex_;
  std::condition_variable cv_;
};
