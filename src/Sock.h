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


class SockFN
{
public:
  virtual ~SockFN() {}
  static FD Connect(struct sockaddr *addr);
  static FD Connect(const char *addr);
};

template<typename _E, typename _Tr = std::char_traits<_E> >
class BasicSock: public SockFN
{
public:
  BasicSock(std::basic_istream<_E, _Tr> *in, std::basic_ostream<_E, _Tr> *out): eofr_(nullptr), eofw_(nullptr), in_((in != nullptr) ? in : &eofr_), out_((out != nullptr) ? out : &eofw_) {}
  BasicSock(int sockfd);
  BasicSock(struct sockaddr *addr);
  BasicSock(const char *addr);
  virtual ~BasicSock();

  virtual bool rdshut() { return in_ == &eofr_; }
  virtual bool wrshut() { return out_ == &eofw_; }

  std::basic_istream<_E, _Tr> &in() { return *in_; }
  std::basic_ostream<_E, _Tr> &out() { return *out_; }
  operator std::basic_istream<_E, _Tr> &() { return in(); }
  operator std::basic_ostream<_E, _Tr> &() { return out(); }

  template<typename _T> std::basic_istream<_E, _Tr> &operator>>(_T _v) { return in() >> _v; }
  template<typename _T> std::basic_ostream<_E, _Tr> &operator<<(_T _v) { return out() << _v; }

  std::streamsize readsome(_E *_s, std::streamsize _n) { return in().readsome(_s, _n); }
  std::basic_istream<_E, _Tr> &read(_E *_s, std::streamsize _n) { return in().read(_s, _n); }
  std::basic_ostream<_E, _Tr> &write(const _E *_s, std::streamsize _n) { return out().write(_s, _n); }

  typename std::basic_istream<_E, _Tr>::int_type get() { return in().get(); }
  std::basic_istream<_E, _Tr> &get(_E &_c) { return in().get(_c); }
  std::basic_istream<_E, _Tr> &putback(_E _c) { return in().putback(_c); }
  std::basic_ostream<_E, _Tr> &put(_E _c) { return out().put(_c); }
  std::basic_ostream<_E, _Tr> &flush() { return out().flush(); }
  virtual std::basic_istream<_E, _Tr> &shutrd();
  virtual std::basic_ostream<_E, _Tr> &shutwr();

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
  FD sfd = Connect(addr);
  _BasicSock((int) sfd);
  sfd.Detach();
}

template<typename _E, typename _Tr>
BasicSock<_E, _Tr>::BasicSock(const char *addr): eofr_(nullptr), eofw_(nullptr), in_(nullptr), out_(nullptr)
{
  FD sfd = Connect(addr);
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
