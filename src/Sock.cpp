#include <stdexcept>
#include <system_error>
#include <unistd.h>
#include <cerrno>
#include <ctime>
#include <cstring>
#include <ctype.h>
#include <alloca.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include "Log.h"
#include "FD.h"
#include "FDStream.h"
#include "Sock.h"

using namespace std;


NetAddrData NetAddr::operator~()
{
  if (family() == AF_UNSPEC)
    logexc << "NetAddr::operator~() failed, this is not initialized" << std::endl;

  NetAddrData rt;
  switch(rt.sa().sa_family = family())
  {
  case AF_INET:
    rt.in().sin_addr.s_addr = ~in().sin_addr.s_addr;
    break;
  case AF_INET6:
    for (int i = 0; i < 16; ++i)
      rt.in6().sin6_addr.s6_addr[i] = ~in6().sin6_addr.s6_addr[i];
    break;
  default:
    logexc << "NetAddr::operator~() failed, unsupported address family" << std::endl;
  }
  return rt;
}

NetAddr &NetAddr::operator&=(const NetAddr &src)
{
  if (family() == AF_UNSPEC)
    logexc << "NetAddr::operator&=() failed, this is not initialized" << std::endl;
  if (family() != src.family())
    logexc << "NetAddr::operator&=() failed, operator value of different address family" << std::endl;

  switch(family())
  {
  case AF_INET:
    in().sin_addr.s_addr &= src.in().sin_addr.s_addr;
    break;
  case AF_INET6:
    for (int i = 0; i < 16; ++i)
      in6().sin6_addr.s6_addr[i] &= src.in6().sin6_addr.s6_addr[i];
    break;
  default:
    logexc << "NetAddr::operator&=() failed, unsupported address family" << std::endl;
  }
  return *this;
}

NetAddr &NetAddr::operator|=(const NetAddr &src)
{
  if (family() == AF_UNSPEC)
    logexc << "NetAddr::operator|=() failed, this is not initialized" << std::endl;
  if (family() != src.family())
    logexc << "NetAddr::operator|=() failed, operator value of different address family" << std::endl;

  switch(family())
  {
  case AF_INET:
    in().sin_addr.s_addr |= src.in().sin_addr.s_addr;
    break;
  case AF_INET6:
    for (int i = 0; i < 16; ++i)
      in6().sin6_addr.s6_addr[i] |= src.in6().sin6_addr.s6_addr[i];
    break;
  default:
    logexc << "NetAddr::operator|=() failed, unsupported address family" << std::endl;
  }
  return *this;
}

bool NetAddr::operator<(const NetAddr &cmp) const
{
  if (family() == AF_UNSPEC)
    logexc << "NetAddr::operator<() failed, this is not initialized" << std::endl;
  if (family() != cmp.family())
    logexc << "NetAddr::operator<() failed, comparator value of different address family" << std::endl;

  switch(family())
  {
  case AF_INET:
    return ntohl(in().sin_addr.s_addr) < ntohl(cmp.in().sin_addr.s_addr);
  case AF_INET6:
    return (::memcmp(&in6().sin6_addr.s6_addr[0], &cmp.in6().sin6_addr.s6_addr[0], 16) < 0);
  default:
    logexc << "NetAddr::operator<() failed, unsupported address family" << std::endl;
    return false;
  }
}

bool NetAddr::operator>(const NetAddr &cmp) const
{
  if (family() == AF_UNSPEC)
    logexc << "NetAddr::operator>() failed, this is not initialized" << std::endl;
  if (family() != cmp.family())
    logexc << "NetAddr::operator>() failed, comparator value of different address family" << std::endl;

  switch(family())
  {
  case AF_INET:
    return ntohl(in().sin_addr.s_addr) > ntohl(cmp.in().sin_addr.s_addr);
  case AF_INET6:
    return (::memcmp(&in6().sin6_addr.s6_addr[0], &cmp.in6().sin6_addr.s6_addr[0], 16) > 0);
  default:
    logexc << "NetAddr::operator>() failed, unsupported address family" << std::endl;
    return false;
  }
}

bool NetAddr::operator==(const NetAddr &cmp) const
{
  if (family() == AF_UNSPEC)
    logexc << "NetAddr::operator<() failed, this is not initialized" << std::endl;
  if (family() != cmp.family())
    logexc << "NetAddr::operator<() failed, comparator value of different address family" << std::endl;

  switch(family())
  {
  case AF_INET:
    return ntohl(in().sin_addr.s_addr) == ntohl(cmp.in().sin_addr.s_addr);
  case AF_INET6:
    return (::memcmp(&in6().sin6_addr.s6_addr[0], &cmp.in6().sin6_addr.s6_addr[0], 16) == 0);
  default:
    logexc << "NetAddr::operator==() failed, unsupported address family" << std::endl;
    return false;
  }
}

NetMask::NetMask(const char *mask)
{
  const char *p;
  size_t l;
  if ((p = strchr(mask, '-')) != nullptr) {
    if (!ScanAddr(mask, p++ - mask, addr_[0]) || !ScanAddr(p, strlen(p), addr_[1]) || (addr_[0].family() != addr_[1].family()))
      logexc << "NetMask::NetMask() failed, bad address(es) in range indication" << std::endl;
    if (addr_[0] >= addr_[1])
      logexc << "NetMask::NetMask() failed, bad range" << std::endl;
  } else if ((p = strchr(mask, '/')) != nullptr) {
    if (!ScanAddr(mask, p++ - mask, addr_[0]))
      logexc << "NetMask::NetMask() failed, bad address" << std::endl;
    if (!ScanCIDR(p, strlen(p), addr_[0], addr_[1]))
      logexc << "NetMask::NetMask() failed, bad CIDR" << std::endl;

  } else if (ScanAddr(mask, l = strlen(mask), addr_[0])) {
    addr_[1] = addr_[0];
  } else {
    if (!ScanName(mask, l, name_))
      logexc << "NetMask::NetMask() failed, empty string" << std::endl;
  }
}


bool NetMask::ScanAddr(const char *buf, size_t len, NetAddr &addr)
{
  const char *q = buf;
  const char *p = buf + len;
  while ((q > p) && isspace(*q))
    ++q;
  while ((q > p) && isspace(p[-1]))
    --p;
  size_t n = p - q;
  char *bf = (char *) memcpy(alloca(n + 1), q, n);
  bf[n] = '\0';

  if (inet_pton(AF_INET, bf, &addr.in()) == 1)
    return true;
  if (inet_pton(AF_INET6, bf, &addr.in6()) == 1)
    return true;
  addr.reset();
  return false;
}

bool NetMask::ScanCIDR(const char *buf, size_t len, const NetAddr &addr, NetAddr &mask)
{
  unsigned cidr = 0, x = 0;
  switch(addr.family())
  {
  case AF_INET:
    x = 32;
    break;
  case AF_INET6:
    x = 128;
    break;
  default:
    logexc << "NetAddr::ScanCIDR() failed, unsupported address family" << std::endl;
  }

  const char *q = buf;
  const char *p = buf + len;
  while ((q > p) && isspace(*q))
    ++q;
  while ((q > p) && isspace(p[-1]))
    --p;
  while (q > p) {
    if (!isdigit(*q))
      return false;
    cidr *= 10;
    cidr += (*q++ - '0');
    if (cidr > x)
      return false;
  }

  switch(mask.sa().sa_family = addr.family())
  {
  case AF_INET:
    mask.in().sin_addr.s_addr = htonl((0xffffffffUL >> (32 - cidr)) << (32 - cidr));
    break;
  case AF_INET6:
    for (x = 0; x < 16; ++x) {
      unsigned d = (cidr >= 8) ? 8 : cidr;
      mask.in6().sin6_addr.s6_addr[x] = (unsigned char) (0xffU << (8 - d));
      cidr -= d;
    }
    break;
  }

  return true;
}

bool NetMask::ScanName(const char *buf, size_t len, std::string &name)
{
  const char *q = buf;
  const char *p = buf + len;
  while ((q > p) && isspace(*q))
    ++q;
  while ((q > p) && isspace(p[-1]))
    --p;
  size_t n = p - q;
  name = std::string(q, n);
  return (n != 0) ? true : false;
}


unsigned SockFN::AddrLen(struct sockaddr *addr)
{
  switch(addr->sa_family)
  {
  case AF_INET:
    return (unsigned) sizeof(struct sockaddr_in);
  case AF_INET6:
    return (unsigned) sizeof(struct sockaddr_in6);
  case AF_LOCAL:
    return (unsigned) SUN_LEN((struct sockaddr_un *) addr);
  default:
    logexc << "unsupported sockaddr::sa_family = " << addr->sa_family << std::endl;
    return -1;  // here we won't come out
  }
}

int SockFN::AddrDomain(struct sockaddr *addr)
{
  switch(addr->sa_family)
  {
  case AF_INET:
    return PF_INET;
  case AF_INET6:
    return PF_INET6;
  case AF_LOCAL:
    return PF_LOCAL;
  default:
    logexc << "unsupported sockaddr::sa_family = " << addr->sa_family << std::endl;
    return -1;  // here we won't come out
  }
}

std::string SockFN::AddrStr(struct sockaddr *addr)
{
  char buf[256];
  std::string s;

  switch(addr->sa_family)
  {
  case AF_INET:
    s = ::inet_ntop(AF_INET, &((struct sockaddr_in *) addr)->sin_addr, buf, sizeof(buf));
    ::sprintf(buf, ":%u", ntohs(((struct sockaddr_in *) addr)->sin_port));
    s += buf;
    break;
  case AF_INET6:
    s = "[";
    s += ::inet_ntop(AF_INET6, &((struct sockaddr_in6 *) addr)->sin6_addr, buf, sizeof(buf));
    s += "]";
    ::sprintf(buf, ":%u", ntohs(((struct sockaddr_in6 *) addr)->sin6_port));
    s += buf;
    break;
  case AF_LOCAL:
    s = ((struct sockaddr_un *) addr)->sun_path;
    break;
  default:
    logexc << "unsupported sockaddr::sa_family = " << addr->sa_family << std::endl;
  }

  return s;
}


FD SockFN::Connect(struct sockaddr *addr)
{
  int domain = AddrDomain(addr);
  unsigned addrlen = AddrLen(addr);

  FD sfd = ::socket(domain, SOCK_STREAM, 0);
  if (sfd == -1)
    logsxc << "socket(domain = " << domain << ") failed" << std::endl;

  while (1) switch (::connect(sfd, addr, addrlen))
  {
  case -1:
    if (errno == EINTR)
      continue;
    if (errno != EISCONN)
      logsxc << "connect(" << AddrStr(addr) << ") failed" << std::endl;
  default:
    return sfd;
  }
}

FD SockFN::Connect(const char *addr)
{
  const char *srvc;
  if ((strchr(addr, '/') != nullptr) || ((srvc = strrchr(addr, ':')) == nullptr)) {
    struct sockaddr_un saddr;
    if (strlen(addr) >= sizeof(saddr.sun_path))
      logexc << "failed to connect \"" << addr << "\", pathname too long" << std::endl;
    saddr.sun_family = AF_LOCAL;
    strcpy(saddr.sun_path, addr);
    return Connect((struct sockaddr *) &saddr);
  }

  std::string straddr(addr, srvc++);
  if ((straddr.size() != 0) && (straddr.front() == '[') && (straddr.back() == ']')) {
    straddr.erase(straddr.begin());
    straddr.pop_back();
  } else if (strchr(straddr.c_str(), ':') != nullptr) {
    logexc << "failed to connect \"" << addr << "\", malformed address" << std::endl;
  }
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

  return sfd;
}

FD SockFN::Listen(struct sockaddr *addr)
{
  int domain = AddrDomain(addr);
  unsigned addrlen = AddrLen(addr);

  FD sfd = ::socket(domain, SOCK_STREAM, 0);
  if (sfd == -1)
    logsxc << "socket(domain = " << domain << ") failed" << std::endl;

  int n = 1;
  ::setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(n));

  if (::bind(sfd, addr, addrlen) < 0)
    logsxc << "bind(" << AddrStr(addr) << ") failed" << std::endl;
  if (::listen(sfd, SOMAXCONN) < 0)
    logsxc << "listen(" << AddrStr(addr) << ") failed" << std::endl;

  return sfd;
}

FD SockFN::Listen(const char *addr)
{
  const char *srvc;
  if ((strchr(addr, '/') != nullptr) || ((srvc = strrchr(addr, ':')) == nullptr)) {
    struct sockaddr_un saddr;
    if (strlen(addr) >= sizeof(saddr.sun_path))
      logexc << "failed to listen on \"" << addr << "\", pathname too long" << std::endl;
    saddr.sun_family = AF_LOCAL;
    strcpy(saddr.sun_path, addr);
    return Listen((struct sockaddr *) &saddr);
  }

  std::string straddr(addr, srvc++);
  if ((straddr.size() != 0) && (straddr.front() == '[') && (straddr.back() == ']')) {
    straddr.erase(straddr.begin());
    straddr.pop_back();
  } else if (strchr(straddr.c_str(), ':') != nullptr) {
    logexc << "failed to listen on \"" << addr << "\", malformed address" << std::endl;
  }
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
  if (aires == nullptr)
    logexc << "failed to listen on \"" << addr << "\", no addrinfo available" << std::endl;

  FD sfd;
  try {
    sfd = Listen(aires->ai_addr);
  } catch(...) {
    ::freeaddrinfo(aires);
    throw;
  }
  ::freeaddrinfo(aires);
  return sfd;
}
