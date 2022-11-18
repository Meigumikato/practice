#include <thread>
#include <future>

class Thread {
public:
    Thread() = default;
    Thread(Thread const& other) = delete;
    Thread(Thread&& other) = default;

    Thread& operator=(Thread const& other) = delete;
    Thread& operator=(Thread&& other) = default;

    ~Thread() {
        join();
    }

    void start() {
        if (!thread_.joinable()) {
            stop_request_ = std::promise<void>();
            thread_ = std::thread(
                &Thread::run, this,
                std::move(stop_request_.get_future())
            );
        }
    }

    void stop() {
        try {
            stop_request_.set_value();
        } catch (std::future_error const& ex) {
            // ignore exception in case of multiple calls to 'stop' function
        }
    }

    void join() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }

protected:
    virtual void run(std::future<void> const& stop_token) = 0;

    static bool is_stop_requested(std::future<void> const& token) noexcept {
        if (token.valid()) {
            auto status = token.wait_for(std::chrono::milliseconds{ 0 });
            if (std::future_status::timeout != status) {
                return true;
            }
        }

        return false;
    }

private:
    std::thread thread_;
    std::promise<void> stop_request_;
};
