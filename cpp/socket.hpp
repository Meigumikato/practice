#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <cerrno>
#include <cstring>

#include <spdlog/spdlog.h>

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

  int GetNativeHandle () {
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

  int bind(::sockaddr_in addr) {
    int r = ::bind(socket_fd_, (::sockaddr*)(&addr), sizeof(addr));
    return r;
  }

  Socket& bind(const char* ip, u_int16_t port) {

    ::sockaddr_in addr;
    memset(&addr, 0,  sizeof(addr));

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
    memset(&addr, 0,  sizeof(addr));
    socklen_t len = sizeof(addr);
    int new_socket_fd = ::accept4(socket_fd_, (::sockaddr*)(&addr), &len, SOCK_CLOEXEC );

    spdlog::info("new connection ip={} port={}", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    return Socket{new_socket_fd};
  }

 private:
  int socket_fd_;
};
