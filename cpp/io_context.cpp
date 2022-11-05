#include <algorithm>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <spdlog/common.h>
#include <stdexcept>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <spdlog/spdlog.h>

#include <cerrno>
#include <cstring>

#include <future>
#include <string>

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

      std::for_each_n(events.begin(), cnt, [](auto& event) {
        Socket* s = reinterpret_cast<Socket*>(event.data.ptr);
        spdlog::info("epoll event socket {}", (int)*s);
      });
    }
  }

  void Register(int fd, Socket& socket) {
    ::epoll_event event;
    event.data.ptr = reinterpret_cast<void*>(&socket);
    event.events = EPOLLIN | EPOLLOUT;

    ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event);
  }

 private:

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

  Acceptor(IoContext& ctx, const char* ip, u_int16_t port) : Acceptor(ip, port) {
    ctx.Register((int)accept_socket_, accept_socket_);
  }

  Socket Accpet() {
    ::sockaddr_in addr;
    memset(&addr, 0,  sizeof(addr));
    socklen_t len = sizeof(addr);
    int new_socket_fd = ::accept4(accept_socket_, (::sockaddr*)(&addr), &len, SOCK_CLOEXEC );

    spdlog::info("new connection ip={} port={}", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    return Socket{new_socket_fd};
  }

  Task<Socket> AsyncAccept() {
    ::sockaddr_in addr;
    memset(&addr, 0,  sizeof(addr));
    socklen_t len = sizeof(addr);
    int new_socket_fd = ::accept4(accept_socket_, (::sockaddr*)(&addr), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    co_return Socket{new_socket_fd};
  }

  Socket GetSocket() {
    return accept_socket_;
  }

 private:
  std::string ip_;
  u_int16_t port_;
  Socket accept_socket_;
};


class Connector {
  public:
    Connector(const char* ip, u_int16_t port) : ip_(ip), port_(port) {
    }

    void Connect() {
      ::sockaddr_in addr;
      memset(&addr, 0,  sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_port = htons(port_);
      addr.sin_addr.s_addr = inet_addr(ip_.c_str());

      socklen_t addrlen = sizeof(::sockaddr);
      int x = ::connect(connect_socket_, (::sockaddr*)(&addr), addrlen);
      if (x == -1) throw;
    }

  private:
    std::string ip_;
    ::sockaddr_in addr_;
    int port_;
    Socket connect_socket_;
};

void Handle(Socket socket) {

  for(;;) {

    std::string data;
    socket.Recv(data);
    if (data == "quit") {
      break;
    }

    socket.Send(data);
  }
}

int main() {

  IoContext ctx;
  Acceptor a(ctx, "127.0.0.1", 8192);

  spdlog::info("wait to accept {}", (int)a.GetSocket());
  spdlog::info("accept in");

  ctx.Run();

  return 0;
}
