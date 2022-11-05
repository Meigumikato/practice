#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <spdlog/spdlog.h>

#include <cerrno>
#include <cstring>

#include <future>
#include <string>

class IoContext {
 public:
  IoContext() {
    epoll_fd_ = ::epoll_create(1024);
  }

  void Run() {

    while (1) {

      epoll_event events[100];
      int cnt = ::epoll_wait(epoll_fd_, events, 100, -1);
      if (cnt == -1) throw;
    }
  }

  void Register() {

  }

 private:
  int epoll_fd_;
};


class Socket {
 public:
  Socket() {
    socket_fd_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
    while (::recv(socket_fd_, buffer, 1024, 0) >= 1024) {
      in_data.append(buffer);
    }
    return 0;
  }

 private:
  int socket_fd_;
};


class Acceptor {
 public:
  Acceptor(const char* ip, u_int16_t port) : accpet_socket_() {

    int r = 0;
    ::sockaddr_in addr;
    memset(&addr, 0,  sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = inet_addr(ip_.c_str());
    socklen_t addrlen = sizeof(::sockaddr);


    r = ::bind(accpet_socket_, (::sockaddr*)(&addr), addrlen);
    if (r == -1) {
      spdlog::error("errno={}", strerror(errno));
      throw errno;
    }

    r = ::listen(accpet_socket_, 100);
    if (r == -1) {
      spdlog::error("errno={}", strerror(errno));
      throw errno;
    }
  }

  Socket Accpet() {
    ::sockaddr_in addr;
    memset(&addr, 0,  sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = inet_addr(ip_.c_str());

    socklen_t addrlen = sizeof(::sockaddr);
    return ::accept4(accpet_socket_, (::sockaddr*)(&addr), &addrlen, SOCK_CLOEXEC);
  }

 private:
  std::string ip_;
  u_int16_t port_;
  Socket accpet_socket_;
};


class Connector {
  public:
    Connector(const char* ip, u_int16_t port) : ip_(ip), port_(port) {
    }

    void Connect() {
      ::sockaddr_in addr;
      memset(&addr, 0,  sizeof(addr));
      addr.sin_family = AF_INET;
      addr.sin_port = htonl(port_);
      addr.sin_addr.s_addr = inet_addr(ip_.c_str());

      socklen_t addrlen = sizeof(::sockaddr);
      int x = ::connect(connect_socket_, (::sockaddr*)(&addr), addrlen);
      if (x == -1) throw;
    }

  private:
    std::string ip_;
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
  Acceptor a("127.0.0.1", 30090);

  for(;;) {
    spdlog::info("wati to accept");
    Socket s = a.Accpet();
    spdlog::info("accept in");
    auto f = std::async(Handle, s);
  }
  return 0;
}
