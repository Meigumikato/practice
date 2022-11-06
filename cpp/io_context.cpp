#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <experimental/net>
#include <experimental/io_context>
#include <source_location>

#include <cerrno>
#include <cstring>

#include <map>
#include <future>
#include <string>
#include <stdexcept>
#include <algorithm>

#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include "task.hpp"

class Socket {
 public:
  Socket() {
    socket_fd_ = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
    if (errno != 0) {
      throw 1;
    }
  }

  Socket(int fd) : socket_fd_(fd) {
  }

  operator int () {
    return socket_fd_;
  }

  int Send(const std::string out_data) {
    return ::send(socket_fd_, out_data.data(), out_data.size(), 0);
  }


  int Recv(std::string& in_data) {
    char buffer[1024];
    int x = ::recv(socket_fd_, buffer, 1024, 0); 
    in_data.append(buffer, x);
    return 0;
  }

  Socket Accpet() {
    ::sockaddr_in addr;
    memset(&addr, 0,  sizeof(addr));
    socklen_t len = sizeof(addr);
    int new_socket_fd = ::accept4(socket_fd_, (::sockaddr*)(&addr), &len, SOCK_CLOEXEC );

    spdlog::info("new connection ip={} port={}", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    return Socket{new_socket_fd};
  }

  Socket AsyncAccpet() {
    ::sockaddr_in addr;
    memset(&addr, 0,  sizeof(addr));
    socklen_t len = sizeof(addr);
    int new_socket_fd = ::accept4(socket_fd_, (::sockaddr*)(&addr), &len, SOCK_CLOEXEC );

    spdlog::info("new connection ip={} port={}", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    return Socket{new_socket_fd};
  }

 private:
  int socket_fd_;
};

class IoContext {
 public:
  IoContext() {
    epoll_fd_ = ::epoll_create(100);
    if (epoll_fd_ == -1) {
      spdlog::throw_spdlog_ex(strerror(errno));
    }
  }

  void Run() {

    std::vector<::epoll_event> events(100);
    while (1) {

      int cnt = ::epoll_wait(epoll_fd_, events.data(), 100, -1);
      if (cnt == -1) throw;

      std::for_each_n(events.begin(), cnt, [this](auto& event) {
        auto h = std::coroutine_handle<>::from_address(event.data.ptr);
        h.resume();
      });
    }
  }

  void Register(int fd, void* ptr) {
    ::epoll_event event;
    event.data.ptr = ptr;
    event.events = EPOLLIN | EPOLLOUT;

    auto ret = ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
    if (ret == -1) {
      spdlog::throw_spdlog_ex(strerror(errno));
    }
  }


  auto Get(Socket socket) & {

    struct Awaiter {
      Awaiter(IoContext* ctx, Socket socket) : ctx(ctx), socket(socket) {}
      bool await_ready() { return false; }
      void await_suspend(std::coroutine_handle<> h) {
        // ctx->frames_.insert({socket, h});
      }

      void await_resume();

      IoContext* ctx;
      Socket socket;
    };

    return Awaiter{this, socket};
  }


 private:
  // std::map<Socket, std::coroutine_handle<>> frames_;
  int epoll_fd_;
};




class Acceptor {
 public:
  Acceptor(const char* ip, u_int16_t port) : ip_(ip), port_(port), accept_socket_() {
    ::sockaddr_in addr;
    int r = 0;
    memset(&addr, 0,  sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(port);
    addr.sin_addr.s_addr = ::inet_addr(ip);

    r = ::bind(accept_socket_, (::sockaddr*)(&addr), sizeof(addr));
    if (r == -1) {
      spdlog::error("errno={}", strerror(errno));
      throw std::logic_error(strerror(errno));
    }

    r = ::listen(accept_socket_, 100);
    if (r == -1) {
      spdlog::error("errno={}", strerror(errno));
      throw std::logic_error(strerror(errno));
    }

    spdlog::info("Accpetor listen on {}:{}", ip, port);
  }

  Acceptor(IoContext* ctx, const char* ip, u_int16_t port) : Acceptor(ip, port) {
    ctx_ = ctx;
  }


  auto AsyncAccpet() {
    struct Awaiter : ::epoll_event {
      IoContext* ctx;
      Socket& socket;

      Awaiter(IoContext* ctx, Socket& socket) : ctx(ctx), socket(socket) {
      }

      bool await_ready() { return false; }

      void await_suspend(std::coroutine_handle<> h) {
        ctx->Register((int)socket, h.address());
      }

      Socket await_resume() {
        return socket.Accpet();
      }
    };

    return Awaiter(ctx_, accept_socket_);
  }

  Socket GetSocket() {
    return accept_socket_;
  }

 private:
  IoContext* ctx_;
  std::string ip_;
  u_int16_t port_;
  Socket accept_socket_;
};


// class Connector {
//   public:
//     Connector(const char* ip, u_int16_t port) : ip_(ip), port_(port) {
//     }
//
//     void Connect() {
//       ::sockaddr_in addr;
//       memset(&addr, 0,  sizeof(addr));
//       addr.sin_family = AF_INET;
//       addr.sin_port = htons(port_);
//       addr.sin_addr.s_addr = inet_addr(ip_.c_str());
//
//       socklen_t addrlen = sizeof(::sockaddr);
//       int x = ::connect(connect_socket_, (::sockaddr*)(&addr), addrlen);
//       if (x == -1) throw;
//     }
//
//   private:
//     std::string ip_;
//     ::sockaddr_in addr_;
//     int port_;
//     Socket connect_socket_;
// };


Task<int> Listener(IoContext* ctx) {

  Acceptor acceptor(ctx, "127.0.0,1", 8192);
  for (;;) {
    auto new_socket = co_await acceptor.AsyncAccpet(); 
    spdlog::info("new socket accept");
  }
}

int main() {

  IoContext ctx;
  // Acceptor a(ctx, "127.0.0.1", 8192);

  // spdlog::info("wait to accept {}", (int)a.GetSocket());
  spdlog::info("accept in");

  auto task = Listener(&ctx);

  ctx.Run();

  return 0;
}
