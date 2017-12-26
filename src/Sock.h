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
#include <memory>
#include "Log.h"
#include "FD.h"
#include "FDStream.h"


class SockAddr
{
public:
  union UData
  {
    struct sockaddr sa;
    struct sockaddr_un un;
    struct sockaddr_in in;
    struct sockaddr_in6 in6;
  };

  SockAddr(UData &data): data_(data) {}
  SockAddr(const SockAddr &) = delete;
  virtual ~SockAddr() {}

  SockAddr &operator=(const SockAddr &src) { memcpy(&data_, &src.data_, sizeof(data_)); return *this; }
  SockAddr &operator=(const sockaddr &src);
  SockAddr &operator=(const char *addr);

  SockAddr &reset() { sa().sa_family = AF_UNSPEC; return *this; }
  int domain() const { return sa().sa_family; }

  struct sockaddr &sa() const { return data_.sa; }
  struct sockaddr_un &un() const { return data_.un; }
  struct sockaddr_in &in() const { return data_.in; }
  struct sockaddr_in6 &in6() const { return data_.in6; }

protected:
  UData &data_;
};

class SockAddrData: public SockAddr
{
public:
  SockAddrData(): SockAddr(data_) { reset(); }
  SockAddrData(const SockAddrData &src): SockAddr(data_) { *this = src; }
  SockAddrData(const SockAddr &src): SockAddr(data_) { SockAddr::operator=(src); }
  SockAddrData(const sockaddr &src): SockAddr(data_) { SockAddr::operator=(src); }
  SockAddrData(const char *addr): SockAddr(data_) { SockAddr::operator=(addr); }
  virtual ~SockAddrData() {}

  SockAddrData &operator=(const SockAddrData &src) { SockAddr::operator=((const SockAddr &) src); return *this; }

protected:
  UData data_;
};


class NetAddrData;
class NetAddr
{
public:
  union UData
  {
    struct sockaddr sa;
    struct sockaddr_in in;
    struct sockaddr_in6 in6;
  };

  NetAddr(UData &data): data_(data) {}
  NetAddr(const NetAddr &) = delete;
  virtual ~NetAddr() {}

  NetAddr &operator=(const NetAddr &src) { memcpy(&data_, &src.data_, sizeof(data_)); return *this; }
  NetAddr &operator=(const sockaddr &src);
  NetAddr &operator=(const char *addr);

  NetAddr &reset() { sa().sa_family = AF_UNSPEC; return *this; }
  int domain() const { return sa().sa_family; }

  struct sockaddr &sa() const { return data_.sa; }
  struct sockaddr_in &in() const { return data_.in; }
  struct sockaddr_in6 &in6() const { return data_.in6; }

  bool iszero();
  NetAddr &setzero();
  NetAddr &setmsbit(unsigned nbits);
  unsigned getmsbit();

  NetAddrData operator~();
  NetAddr &operator&=(const NetAddr &src);
  NetAddrData operator&(const NetAddr &src);
  NetAddr &operator|=(const NetAddr &src);
  NetAddrData operator|(const NetAddr &src);
  NetAddr &operator^=(const NetAddr &src);
  NetAddrData operator^(const NetAddr &src);
  bool operator<(const NetAddr &cmp) const;
  bool operator>(const NetAddr &cmp) const;
  bool operator<=(const NetAddr &cmp) const { return !operator>(cmp); }
  bool operator>=(const NetAddr &cmp) const { return !operator<(cmp); }
  bool operator==(const NetAddr &cmp) const;
  bool operator!=(const NetAddr &cmp) const { return !operator==(cmp); }

  bool operator<(const sockaddr &cmp) const { return operator<(NetAddr((UData &) cmp)); }
  bool operator>(const sockaddr &cmp) const { return operator>(NetAddr((UData &) cmp)); }
  bool operator<=(const sockaddr &cmp) const { return operator<=(NetAddr((UData &) cmp)); }
  bool operator>=(const sockaddr &cmp) const { return operator>=(NetAddr((UData &) cmp)); }
  bool operator==(const sockaddr &cmp) const { return operator==(NetAddr((UData &) cmp)); }
  bool operator!=(const sockaddr &cmp) const { return operator!=(NetAddr((UData &) cmp)); }

protected:
  UData &data_;
};

class NetAddrData: public NetAddr
{
public:
  NetAddrData(): NetAddr(data_) { reset(); }
  NetAddrData(const NetAddrData &src): NetAddr(data_) { *this = src; }
  NetAddrData(const NetAddr &src): NetAddr(data_) { NetAddr::operator=(src); }
  NetAddrData(const sockaddr &src): NetAddr(data_) { NetAddr::operator=(src); }
  NetAddrData(const char *addr): NetAddr(data_) { NetAddr::operator=(addr); }
  virtual ~NetAddrData() {}

  NetAddrData &operator=(const NetAddrData &src) { NetAddr::operator=((const NetAddr &) src); return *this; }

protected:
  UData data_;
};


class NetMask
{
public:
  NetMask() { reset(); }
  NetMask(const char *mask) { *this = mask; }
  virtual ~NetMask() {}

  NetMask &operator=(const char *mask);
  NetMask &reset();
  bool valid() const;

  const std::string &name() const { return name_; }
  const NetAddrData &addr(int i) const { return addr_[i]; }

  bool match(const NetAddr &addr) const;

protected:
  std::string name_;
  NetAddrData addr_[2];

private:
  static bool ScanAddr(const char *buf, size_t len, NetAddr &addr);
  static bool ScanCIDR(const char *buf, size_t len, const NetAddr &addr, NetAddr &mask);
  static bool ScanName(const char *buf, size_t len, std::string &name);
};


std::ostream &operator<<(std::ostream &os, const struct sockaddr &addr);
inline std::ostream &operator<<(std::ostream &os, const SockAddr &addr) { return os << addr.sa(); }
std::ostream &operator<<(std::ostream &os, const NetAddr &addr);
std::ostream &operator<<(std::ostream &os, const NetMask &netmsk);


class SockFN
{
public:
  virtual ~SockFN() {}
  static unsigned AddrLen(struct sockaddr *addr);
  static int AddrDomain(struct sockaddr *addr);
  static std::string AddrStr(struct sockaddr *addr);
  static FD Connect(struct sockaddr *addr);
  static FD Connect(const char *addr);
  static FD Listen(struct sockaddr *addr);
  static FD Listen(const char *addr);
};


template<typename _E, typename _Tr = std::char_traits<_E> >
class BasicSock:
  public SockFN,
  private std::unique_ptr<std::basic_istream<_E, _Tr> >,
  private std::unique_ptr<std::basic_ostream<_E, _Tr> >
{
public:
  BasicSock():
    eofr_(nullptr), eofw_(nullptr),
    in_(&eofr_), out_(&eofw_) {}
  BasicSock(std::basic_streambuf<_E, _Tr> *isb, std::basic_streambuf<_E, _Tr> *osb):
    std::unique_ptr<std::basic_istream<_E, _Tr> >((isb != nullptr) ? new std::basic_istream<_E, _Tr>(nullptr) : nullptr),
    std::unique_ptr<std::basic_ostream<_E, _Tr> >((osb != nullptr) ? new std::basic_ostream<_E, _Tr>(nullptr) : nullptr),
    eofr_(nullptr), eofw_(nullptr),
    in_((isb != nullptr) ? std::unique_ptr<std::basic_istream<_E, _Tr> >::get() : &eofr_),
    out_((osb != nullptr) ? std::unique_ptr<std::basic_ostream<_E, _Tr> >::get() : &eofw_)
  {
    if (isb != nullptr)
      in_->rdbuf(isb);
    if (osb != nullptr)
      out_->rdbuf(osb);
  }
  BasicSock(std::basic_istream<_E, _Tr> *in, std::basic_ostream<_E, _Tr> *out):
    std::unique_ptr<std::basic_istream<_E, _Tr> >(in),
    std::unique_ptr<std::basic_ostream<_E, _Tr> >(out),
    eofr_(nullptr), eofw_(nullptr),
    in_((in != nullptr) ? in : &eofr_), out_((out != nullptr) ? out : &eofw_) {}
  BasicSock(int sockfd);
  BasicSock(FD &&sfd): BasicSock((int) sfd) { sfd.Detach(); }
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
  std::unique_ptr<std::basic_istream<_E, _Tr> >::reset();
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
  std::unique_ptr<std::basic_ostream<_E, _Tr> >::reset();
  return *(out_ = &eofw_);
}


#endif // SOCK_H_INCLUDED
