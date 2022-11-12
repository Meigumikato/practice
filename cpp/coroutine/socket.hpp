#include <arpa/inet.h>
#include <netinet/in.h>
#include <spdlog/spdlog.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cerrno>
#include <cstring>

#include "io_context.hpp"

class Socket {
 public:
  Socket(IoContext& io_context) : 
    socket_fd_(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)),
    io_context_(&io_context) {
    if (errno != 0) {
      throw 1;
    }
  }

  Socket(int fd, IoContext& io_context) : socket_fd_(fd), io_context_(&io_context) {}

  int GetNativeHandle() { return socket_fd_; }

  int Send(const std::string out_data) {
    return ::send(socket_fd_, out_data.data(), out_data.size(), 0);
  }

  int Recv(std::string& in_data) {
    char buffer[1024];
    int x = ::recv(socket_fd_, buffer, 1024, 0);
    in_data.append(buffer, x);
    return 0;
  }

  int bind(::sockaddr_in addr) {
    int r = ::bind(socket_fd_, (::sockaddr*)(&addr), sizeof(addr));
    return r;
  }

  Socket& bind(const char* ip, u_int16_t port) {
    ::sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = ::htons(port);
    addr.sin_addr.s_addr = ::inet_addr(ip);

    int r = ::bind(socket_fd_, (::sockaddr*)(&addr), sizeof(addr));
    if (r != 0) {
      spdlog::spdlog_ex(fmt::format("socket bind failed {}", strerror(errno)));
    }

    return *this;
  }

  Socket& listen(int max_queue_size) {
    int r = ::listen(socket_fd_, max_queue_size);
    if (r != 0) {
      spdlog::spdlog_ex(fmt::format("socket listen failed {}", strerror(errno)));
    }
    return *this;
  }

  Socket Accpet() {
    ::sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t len = sizeof(addr);
    int new_socket_fd = ::accept4(socket_fd_, (::sockaddr*)(&addr), &len, SOCK_CLOEXEC);

    spdlog::info("new connection ip={} port={}", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    return Socket{new_socket_fd, *io_context_};
  }


  auto AsyncAccpet() {
    struct Awaiter {
      auto await_ready() { return false; }
      auto await_suspend(std::coroutine_handle<> h) {
        ::epoll_event edata;
        edata.events = EPOLLIN;
        io_context->Register(&edata, h);
      }

      Socket await_resume() {
        return server->Accpet();
      }

      Socket* server;
      IoContext* io_context;
    };
  }

 private:
  int socket_fd_;
  IoContext* io_context_;
};
