#ifndef SOCK_H_INCLUDED
#define SOCK_H_INCLUDED

#include <stdexcept>
#include <system_error>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <istream>
#include <ostream>
#include <iostream>
#include <iosfwd>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include "Log.h"
#include "FD.h"
#include "FDStream.h"


template<typename _E, typename _Tr = std::char_traits<_E> >
class BasicSock
{
public:
  BasicSock(std::basic_istream<_E, _Tr> *in, std::basic_ostream<_E, _Tr> *out): eofr_(nullptr), eofw_(nullptr), in_((in != nullptr) ? in : &eofr_), out_((out != nullptr) ? out : &eofw_) {}
  BasicSock(int sockfd);
  BasicSock(struct sockaddr *addr);
  BasicSock(const char *addr);
  virtual ~BasicSock();

  bool rdshut() { return in_ == &eofr_; }
  bool wrshut() { return out_ == &eofw_; }

  std::basic_istream<_E, _Tr> &in() { return *in_; }
  std::basic_ostream<_E, _Tr> &out() { return *out_; }
  operator std::basic_istream<_E, _Tr> &() { return in(); }
  operator std::basic_ostream<_E, _Tr> &() { return out(); }

  template<typename _T> std::basic_istream<_E, _Tr> &operator>>(_T _v) { return in() >> _v; }
  template<typename _T> std::basic_ostream<_E, _Tr> &operator<<(_T _v) { return out() << _v; }

  std::basic_istream<_E, _Tr> &read(_E *_s, std::streamsize _n) { return in().read(_s, _n); }
  std::basic_ostream<_E, _Tr> &write(const _E *_s, std::streamsize _n) { return out().write(_s, _n); }

  typename std::basic_istream<_E, _Tr>::int_type get() { return in().get(); }
  std::basic_istream<_E, _Tr> &get(_E &_c) { return in().get(_c); }
  std::basic_istream<_E, _Tr> &putback(_E _c) { return in().putback(_c); }
  std::basic_ostream<_E, _Tr> &put(_E _c) { return out().put(_c); }
  std::basic_ostream<_E, _Tr> &flush() { return out().flush(); }
  std::basic_istream<_E, _Tr> &shutrd();
  std::basic_ostream<_E, _Tr> &shutwr();

private:
  void _BasicSock(int sockfd);
  void _BasicSock(struct sockaddr *addr);

  std::basic_istream<_E, _Tr> eofr_;
  std::basic_ostream<_E, _Tr> eofw_;

  std::basic_istream<_E, _Tr> *in_;
  std::basic_ostream<_E, _Tr> *out_;
};

typedef BasicSock<char> Sock;
typedef BasicSock<wchar_t> WSock;


template<typename _E, typename _Tr>
BasicSock<_E, _Tr>::BasicSock(int sockfd): eofr_(nullptr), eofw_(nullptr), in_(nullptr), out_(nullptr) { _BasicSock(sockfd); }

template<typename _E, typename _Tr>
void BasicSock<_E, _Tr>::_BasicSock(int sockfd)
{
  FD fd2 = ::dup(sockfd);
  if (fd2 == -1)
    logsxc << "dup(sockfd) failed" << std::endl;
  out_ = new FDStream(fd2, std::ios::out);
  fd2.Detach();
  in_ = new FDStream(sockfd, std::ios::in);
}

template<typename _E, typename _Tr>
BasicSock<_E, _Tr>::BasicSock(struct sockaddr *addr): eofr_(nullptr), eofw_(nullptr), in_(nullptr), out_(nullptr) { _BasicSock(addr); }

template<typename _E, typename _Tr>
void BasicSock<_E, _Tr>::_BasicSock(struct sockaddr *addr)
{
  int domain;
  unsigned addrlen;
  switch(addr->sa_family)
  {
  case AF_INET:
    domain = PF_INET;
    addrlen = sizeof(struct sockaddr_in);
    break;
  case AF_INET6:
    domain = PF_INET6;
    addrlen = sizeof(struct sockaddr_in6);
    break;
  case AF_LOCAL:
    domain = PF_LOCAL;
    addrlen = SUN_LEN((struct sockaddr_un *) addr);
    break;
  default:
    logerr << "unsupported sockaddr::sa_family = " << addr->sa_family << std::endl;
    throw std::invalid_argument("sockaddr::sa_family");
  }

  FD sfd = ::socket(domain, SOCK_STREAM, 0);
  if (sfd == -1)
    logsxc << "socket(domain = " << domain << ") failed" << std::endl;

  while (1) switch (::connect(sfd, addr, addrlen))
  {
  case -1:
    if (errno == EINTR)
      continue;
    if (errno != EISCONN)
      logsxc << "connect() failed" << std::endl;
  default:
    _BasicSock((int) sfd);
    sfd.Detach();
    return;
  }
}

template<typename _E, typename _Tr>
BasicSock<_E, _Tr>::BasicSock(const char *addr): eofr_(nullptr), eofw_(nullptr), in_(nullptr), out_(nullptr)
{
  const char *srvc;
  if ((strchr(addr, '/') != nullptr) || ((srvc = strrchr(addr, ':')) == nullptr)) {
    struct sockaddr_un saddr;
    saddr.sun_family = AF_LOCAL;
    strncpy(saddr.sun_path, addr, sizeof(saddr.sun_path) - 1);
    saddr.sun_path[sizeof(saddr.sun_path) - 1] = '\0';
    _BasicSock((struct sockaddr *) &saddr);
    return;
  }

  std::string straddr(addr, srvc++);
  struct addrinfo *aires, hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  errno = 0;
  int airt = ::getaddrinfo(straddr.c_str(), srvc, &hints, &aires);
  if (airt != 0) {
    if (errno == 0)
      errno = ENOENT;
    logsxc << "getaddrinfo(addr = " << straddr << ", srvc = " << srvc <<") failed: " << gai_strerror(airt) << std::endl;
  }

  FD sfd;
  errno = 0;
  for (struct addrinfo *ai = aires; (sfd == -1) && (ai != nullptr); ai = ai->ai_next) {
    sfd = ::socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (sfd == -1) {
      logerr << "socket(domain = " << ai->ai_family << ", type = " << ai->ai_socktype << ", proto = " << ai->ai_protocol << ") failed with " << errno << std::endl;
      continue;
    }
    while (::connect(sfd, ai->ai_addr, ai->ai_addrlen) == -1) {
      if (errno == EINTR)
        continue;
      if (errno != EISCONN) {
        logerr << "connect(addr = " << addr << ", domain = " << ai->ai_family << ", type = " << ai->ai_socktype << ", proto = " << ai->ai_protocol << ") failed with " << errno << std::endl;
        sfd.Close();
      }
      break;
    }
  }
  if (aires != nullptr)
    ::freeaddrinfo(aires);

  if (sfd == -1) {
    if (errno == 0) {
      logexc << "failed to connect \"" << addr << "\", no addrinfo available" << std::endl;
    } else {
      logsxc << "failed to connect \"" << addr << "\"" << std::endl;
    }
  }

  _BasicSock((int) sfd);
  sfd.Detach();
}

template<typename _E, typename _Tr>
BasicSock<_E, _Tr>::~BasicSock()
{
  if (!wrshut())
    shutwr();
  if (!rdshut())
    shutrd();
}

template<typename _E, typename _Tr>
std::basic_istream<_E, _Tr> &BasicSock<_E, _Tr>::shutrd()
{
  if (rdshut())
    return *in_;

  BasicFDStream<_E, _Tr> *fds = dynamic_cast< BasicFDStream<_E, _Tr> * >(in_);
  if (fds != nullptr) {
    struct stat st;
    if (::fstat(fds->fd(), &st) == 0) {
      if (S_ISSOCK(st.st_mode) && (::shutdown(fds->fd(), SHUT_RD) == -1) && (errno != ENOTCONN)) {
        logerr << "shutdown(SHUT_RD) failed with " << errno << std::endl;
        eofr_.setstate(std::ios::failbit);
      }
    } else {
      logerr << "fstat() failed with " << errno << std::endl;
      eofr_.setstate(std::ios::failbit);
    }
  }
  delete in_;
  return *(in_ = &eofr_);
}

template<typename _E, typename _Tr>
std::basic_ostream<_E, _Tr> &BasicSock<_E, _Tr>::shutwr()
{
  if (wrshut())
    return *out_;

  BasicFDStream<_E, _Tr> *fds = dynamic_cast< BasicFDStream<_E, _Tr> * >(out_);
  if (fds != nullptr) {
    struct stat st;
    if (::fstat(fds->fd(), &st) == 0) {
      if (S_ISSOCK(st.st_mode) && (::shutdown(fds->fd(), SHUT_WR) == -1)) {
        logerr << "shutdown(SHUT_WR) failed with " << errno << std::endl;
        eofw_.setstate(std::ios::failbit);
      }
    } else {
      logerr << "fstat() failed with " << errno << std::endl;
      eofw_.setstate(std::ios::failbit);
    }
  }
  delete out_;
  return *(out_ = &eofw_);
}


#endif // SOCK_H_INCLUDED
