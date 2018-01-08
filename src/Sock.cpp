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


SockAddr &SockAddr::operator=(const sockaddr &src)
{
  switch(src.sa_family)
  {
  case AF_INET:
    in().sin_family = AF_INET;
    in().sin_addr.s_addr = ((const struct sockaddr_in &) src).sin_addr.s_addr;
    break;
  case AF_INET6:
    in6().sin6_family = AF_INET6;
    for (int i = 0; i < 16; ++i)
      in6().sin6_addr.s6_addr[i] = ((const struct sockaddr_in6 &) src).sin6_addr.s6_addr[i];
    break;
  case AF_LOCAL:
    un().sun_family = AF_LOCAL;
    strncpy(un().sun_path, ((const struct sockaddr_un &) src).sun_path, sizeof(((const struct sockaddr_un &) src).sun_path));
    break;
  default:
    logexc << "SockAddr::operator=(const sockaddr &) failed, unsupported address family" << std::endl;
  }
  return *this;
}

SockAddr &SockAddr::operator=(const char *addr)
{
  const char *srvc;
  if ((strchr(addr, '/') != nullptr) || (((srvc = strrchr(addr, ':')) == nullptr) && svc_.empty())) {
    if (strlen(addr) >= sizeof(un().sun_path))
      logexc << "SockAddr::operator=(\"" << addr << "\") failed, pathname too long" << std::endl;
    un().sun_family = AF_LOCAL;
    strcpy(un().sun_path, addr);
    return *this;
  }

  std::string straddr(addr, (srvc != nullptr) ? srvc : (const char *) (addr + strlen(addr)));
  if (srvc != nullptr) {
    ++srvc;
    if ((straddr.size() != 0) && (straddr.front() == '[') && (straddr.back() == ']')) {
      straddr.erase(straddr.begin());
      straddr.pop_back();
    } else if (strchr(straddr.c_str(), ':') != nullptr) {
      if (!svc_.empty()) {
        straddr = addr;
        srvc = svc_.c_str();
      } else {
        logexc << "SockAddr::operator=(\"" << addr << "\") failed, malformed address" << std::endl;
      }
    }
  } else {
    srvc = svc_.c_str();
  }
  struct addrinfo *aires, hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_ADDRCONFIG | AI_ALL;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  errno = 0;
  int airt = ::getaddrinfo(straddr.c_str(), (strcmp(srvc, "*") != 0) ? srvc : (srvc = nullptr), &hints, &aires);
  if (airt != 0) {
    if (errno == 0)
      errno = ENOENT;
    logsxc << "SockAddr::operator=(\"" << addr << "\") failed: " << gai_strerror(airt) << std::endl;
  }
  if (aires == nullptr)
    logexc << "SockAddr::operator=(\"" << addr << "\") failed, no addrinfo available" << std::endl;

  for (struct addrinfo *ai = aires; ai != nullptr; ai = ai->ai_next) switch (ai->ai_family) { // AF_INET takes precedence
  case AF_INET:
    *this = *aires->ai_addr;
    if (srvc == nullptr)
      in().sin_port = 0;
    ::freeaddrinfo(aires);
    return *this;
  }
  for (struct addrinfo *ai = aires; ai != nullptr; ai = ai->ai_next) switch (ai->ai_family) {
  case AF_INET6:
    *this = *aires->ai_addr;
    if (srvc == nullptr)
      in6().sin6_port = 0;
    ::freeaddrinfo(aires);
    return *this;
  }
  ::freeaddrinfo(aires);
  logexc << "SockAddr::operator=(\"" << addr << "\") failed, no suitable addrinfo available" << std::endl;
  return *this;
}

unsigned SockAddr::port() const
{
  if (domain() == AF_UNSPEC)
    logexc << "SockAddr::port() failed, this is not initialized" << std::endl;

  switch(domain())
  {
  case AF_INET:
    return ntohs(in().sin_port);
  case AF_INET6:
    return ntohs(in6().sin6_port);
  default:
    logexc << "SockAddr::port() failed, unsupported address family" << std::endl;
    return 0;
  }
}

SockAddr &SockAddr::port(unsigned port)
{
  if (domain() == AF_UNSPEC)
    logexc << "SockAddr::port(" << port << ") failed, this is not initialized" << std::endl;

  switch(domain())
  {
  case AF_INET:
    in().sin_port = htons(port);
    break;
  case AF_INET6:
    in6().sin6_port = htons(port);
    break;
  default:
    logexc << "SockAddr::port(" << port << ") failed, unsupported address family" << std::endl;
  }
  return *this;
}


NetAddr &NetAddr::operator=(const sockaddr &src)
{
  switch(src.sa_family)
  {
  case AF_INET:
    in().sin_family = AF_INET;
    in().sin_addr.s_addr = ((const struct sockaddr_in &) src).sin_addr.s_addr;
    break;
  case AF_INET6:
    in6().sin6_family = AF_INET6;
    for (int i = 0; i < 16; ++i)
      in6().sin6_addr.s6_addr[i] = ((const struct sockaddr_in6 &) src).sin6_addr.s6_addr[i];
    break;
  default:
    logexc << "NetAddr::operator=(const sockaddr &) failed, unsupported address family" << std::endl;
  }
  return *this;
}

NetAddr &NetAddr::operator=(const char *addr)
{
  if (!dns_) {
    if (inet_pton(AF_INET, addr, &in().sin_addr) == 1)
      return in().sin_family = AF_INET, *this;
    if (inet_pton(AF_INET6, addr, &in6().sin6_addr) == 1)
      return in6().sin6_family = AF_INET6, *this;
    logexc << "NetAddr::operator=(\"" << addr << "\") failed, bad address" << std::endl;
    return *this;
  }

  struct addrinfo *aires, hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_ADDRCONFIG | AI_ALL;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  errno = 0;
  int airt = ::getaddrinfo(addr, NULL, &hints, &aires);
  if (airt != 0) {
    if (errno == 0)
      errno = ENOENT;
    logsxc << "NetAddr::operator=(\"" << addr << "\") failed: " << gai_strerror(airt) << std::endl;
  }
  if (aires == nullptr)
    logexc << "NetAddr::operator=(\"" << addr << "\") failed, no addrinfo available" << std::endl;

  for (struct addrinfo *ai = aires; ai != nullptr; ai = ai->ai_next) switch (ai->ai_family) { // AF_INET takes precedence
  case AF_INET:
    *this = *aires->ai_addr;
    ::freeaddrinfo(aires);
    return *this;
  }
  for (struct addrinfo *ai = aires; ai != nullptr; ai = ai->ai_next) switch (ai->ai_family) {
  case AF_INET6:
    *this = *aires->ai_addr;
    ::freeaddrinfo(aires);
    return *this;
  }
  ::freeaddrinfo(aires);
  logexc << "NetAddr::operator=(\"" << addr << "\") failed, no suitable addrinfo available" << std::endl;
  return *this;
}

bool NetAddr::iszero()
{
  if (domain() == AF_UNSPEC)
    logexc << "NetAddr::iszero() failed, this is not initialized" << std::endl;

  switch(domain())
  {
  case AF_INET:
    return (in().sin_addr.s_addr == 0UL);
  case AF_INET6:
    for (unsigned x = 0; x < 16; ++x) {
      if (in6().sin6_addr.s6_addr[x])
        return false;
    }
    return true;
  default:
    logexc << "NetAddr::iszero() failed, unsupported address family" << std::endl;
    return false;
  }
}

NetAddr &NetAddr::setzero()
{
  if (domain() == AF_UNSPEC)
    logexc << "NetAddr::setzero() failed, this is not initialized" << std::endl;

  switch(domain())
  {
  case AF_INET:
    in().sin_addr.s_addr = 0;
    break;
  case AF_INET6:
    for (unsigned x = 0; x < 16; ++x)
      in6().sin6_addr.s6_addr[x] = 0;
    break;
  default:
    logexc << "NetAddr::setzero() failed, unsupported address family" << std::endl;
  }
  return *this;
}

NetAddr &NetAddr::setzero(int domain)
{
  switch(domain)
  {
  case AF_INET:
    in().sin_family = domain;
    in().sin_addr.s_addr = 0;
    break;
  case AF_INET6:
    in6().sin6_family = domain;
    for (unsigned x = 0; x < 16; ++x)
      in6().sin6_addr.s6_addr[x] = 0;
    break;
  default:
    logexc << "NetAddr::setzero() failed, unsupported domain" << std::endl;
  }
  return *this;
}

NetAddr &NetAddr::setmsbit(unsigned nbits)
{
  if (domain() == AF_UNSPEC)
    logexc << "NetAddr::setmsbit() failed, this is not initialized" << std::endl;

  switch(domain())
  {
  case AF_INET:
    if (nbits > 32)
      logexc << "NetAddr::setmsbit() failed, nbits invalid" << std::endl;
    in().sin_addr.s_addr |= htonl((0xffffffffUL >> (32 - nbits)) << (32 - nbits));
    break;
  case AF_INET6:
    if (nbits > 128)
      logexc << "NetAddr::setmsbit() failed, nbits invalid" << std::endl;
    for (unsigned x = 0; x < 16; ++x) {
      unsigned d = (nbits >= 8) ? 8 : nbits;
      in6().sin6_addr.s6_addr[x] |= (unsigned char) (0xffU << (8 - d));
      nbits -= d;
    }
    break;
  default:
    logexc << "NetAddr::setmsbit() failed, unsupported address family" << std::endl;
  }
  return *this;
}

unsigned NetAddr::getmsbit()
{
  if (domain() == AF_UNSPEC)
    logexc << "NetAddr::getmsbit() failed, this is not initialized" << std::endl;

  unsigned nbits = 0;
  switch(domain())
  {
  case AF_INET:
    {
      unsigned long mask = ntohl(in().sin_addr.s_addr);
      unsigned long b = 0x80000000UL;
      while ((mask & b) != 0UL) {
        ++nbits;
        b >>= 1;
      }
    }
    break;
  case AF_INET6:
    {
      for (unsigned x = 0; x < 16; ++x) {
        unsigned char mask = in6().sin6_addr.s6_addr[x];
        if (!((unsigned char) ~mask)) {
          nbits += 8;
          continue;
        }
        unsigned char b = 0x80U;
        while (mask & b) {
          ++nbits;
          b >>= 1;
        }
        break;
      }
    }
    break;
  default:
    logexc << "NetAddr::getmsbit() failed, unsupported address family" << std::endl;
  }
  return nbits;
}

NetAddrData NetAddr::operator~()
{
  if (domain() == AF_UNSPEC)
    logexc << "NetAddr::operator~() failed, this is not initialized" << std::endl;

  NetAddrData rt;
  switch(rt.sa().sa_family = domain())
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

NetAddrData NetAddr::operator&(const NetAddr &src)
{
  NetAddrData rt = *this;
  rt &= src;
  return rt;
}

NetAddrData NetAddr::operator|(const NetAddr &src)
{
  NetAddrData rt = *this;
  rt |= src;
  return rt;
}

NetAddrData NetAddr::operator^(const NetAddr &src)
{
  NetAddrData rt = *this;
  rt ^= src;
  return rt;
}

NetAddr &NetAddr::operator&=(const NetAddr &src)
{
  if (domain() == AF_UNSPEC)
    logexc << "NetAddr::operator&=() failed, this is not initialized" << std::endl;
  if (domain() != src.domain())
    logexc << "NetAddr::operator&=() failed, operator value of different address family" << std::endl;

  switch(domain())
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
  if (domain() == AF_UNSPEC)
    logexc << "NetAddr::operator|=() failed, this is not initialized" << std::endl;
  if (domain() != src.domain())
    logexc << "NetAddr::operator|=() failed, operator value of different address family" << std::endl;

  switch(domain())
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

NetAddr &NetAddr::operator^=(const NetAddr &src)
{
  if (domain() == AF_UNSPEC)
    logexc << "NetAddr::operator^=() failed, this is not initialized" << std::endl;
  if (domain() != src.domain())
    logexc << "NetAddr::operator^=() failed, operator value of different address family" << std::endl;

  switch(domain())
  {
  case AF_INET:
    in().sin_addr.s_addr ^= src.in().sin_addr.s_addr;
    break;
  case AF_INET6:
    for (int i = 0; i < 16; ++i)
      in6().sin6_addr.s6_addr[i] ^= src.in6().sin6_addr.s6_addr[i];
    break;
  default:
    logexc << "NetAddr::operator^=() failed, unsupported address family" << std::endl;
  }
  return *this;
}

bool NetAddr::operator<(const NetAddr &cmp) const
{
  if (domain() == AF_UNSPEC)
    logexc << "NetAddr::operator<() failed, this is not initialized" << std::endl;
  if (domain() != cmp.domain())
    logexc << "NetAddr::operator<() failed, comparator value of different address family" << std::endl;

  switch(domain())
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
  if (domain() == AF_UNSPEC)
    logexc << "NetAddr::operator>() failed, this is not initialized" << std::endl;
  if (domain() != cmp.domain())
    logexc << "NetAddr::operator>() failed, comparator value of different address family" << std::endl;

  switch(domain())
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
  if (domain() == AF_UNSPEC)
    logexc << "NetAddr::operator==() failed, this is not initialized" << std::endl;
  if (domain() != cmp.domain())
    logexc << "NetAddr::operator==() failed, comparator value of different address family" << std::endl;

  switch(domain())
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

NetAddrData::NetAddrData(): NetAddr(*((UData *) memset(&data_, 0, sizeof(data_))))
{
  reset();
}


NetMask &NetMask::operator=(const NetMask &src)
{
  name_ = src.name_;
  addr_[0] = src.addr_[0];
  addr_[1] = src.addr_[1];
  return *this;
}

NetMask &NetMask::operator=(const char *mask)
{
  const char *p;
  size_t l;
  if ((p = strchr(mask, '-')) != nullptr) {
    if (!ScanAddr(mask, p++ - mask, addr_[0]) || !ScanAddr(p, strlen(p), addr_[1]) || (addr_[0].domain() != addr_[1].domain()))
      logexc << "NetMask::NetMask() failed, bad address(es)" << std::endl;
    if (addr_[0] > addr_[1])
      logexc << "NetMask::NetMask() failed, bad range" << std::endl;
    name_.clear();
  } else if ((p = strchr(mask, '/')) != nullptr) {
    if (!ScanAddr(mask, p++ - mask, addr_[0]))
      logexc << "NetMask::NetMask() failed, bad address" << std::endl;
    if (!ScanCIDR(p, strlen(p), addr_[0], addr_[1]))
      logexc << "NetMask::NetMask() failed, bad CIDR" << std::endl;
    if (addr_[0] != (addr_[0] & addr_[1]))
      logexc << "NetMask::NetMask() failed, address does not fit in mask" << std::endl;
    addr_[1] =  ~addr_[1];
    addr_[1] |= addr_[0];
    name_.clear();
  } else if (ScanAddr(mask, l = strlen(mask), addr_[0])) {
    addr_[1] = addr_[0];
    name_.clear();
  } else {
    if (!ScanName(mask, l, name_))
      logexc << "NetMask::NetMask() failed, empty string" << std::endl;
    addr_[0].reset();
    addr_[1].reset();
  }
  return *this;
}

NetMask &NetMask::reset()
{
  name_.clear();
  addr_[0].reset();
  addr_[1].reset();
  return *this;
}

bool NetMask::valid() const
{
  return !name().empty()
    ? ((addr_[0].domain() == AF_UNSPEC) && (addr_[1].domain() == AF_UNSPEC))
    : ((addr_[0].domain() != AF_UNSPEC) && (addr_[1].domain() ==  addr_[0].domain()) && (addr_[1] >= addr_[0]));
}

bool NetMask::match(const NetAddr &addr) const
{
  if (!name_.empty()) {
    if (name_ == "*")
      return true;

    struct addrinfo *aires, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_ADDRCONFIG | AI_ALL;
    hints.ai_family = addr.domain();
    hints.ai_socktype = SOCK_STREAM;
    int airt = ::getaddrinfo(name_.c_str(), nullptr, &hints, &aires);
    if (airt != 0)
      return false;

    bool rt = false;
    for (struct addrinfo *ai = aires; ai != nullptr; ai = ai->ai_next)
      rt |= (addr == *ai->ai_addr);
    if (aires != nullptr)
      ::freeaddrinfo(aires);
    return rt;
  }

  return ((addr.domain() == addr_[0].domain()) && (addr >= addr_[0]) && (addr <= addr_[1]));
}


bool NetMask::ScanAddr(const char *buf, size_t len, NetAddr &addr)
{
  const char *q = buf;
  const char *p = buf + len;
  while ((q < p) && isspace(*q))
    ++q;
  while ((p > q) && isspace(p[-1]))
    --p;
  size_t n = p - q;
  char *bf = (char *) memcpy(alloca(n + 1), q, n);
  bf[n] = '\0';

  if (inet_pton(AF_INET, bf, &addr.in().sin_addr) == 1)
    return addr.in().sin_family = AF_INET, true;
  if (inet_pton(AF_INET6, bf, &addr.in6().sin6_addr) == 1)
    return addr.in6().sin6_family = AF_INET6, true;
  addr.reset();
  return false;
}

bool NetMask::ScanCIDR(const char *buf, size_t len, const NetAddr &addr, NetAddr &mask)
{
  unsigned cidr = 0, x = 0;
  switch(addr.domain())
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
  while ((q < p) && isspace(*q))
    ++q;
  while ((p > q) && isspace(p[-1]))
    --p;
  while (q < p) {
    if (!isdigit(*q))
      return false;
    cidr *= 10;
    cidr += (*q++ - '0');
    if (cidr > x)
      return false;
  }

  mask.setzero(addr.domain());
  mask.setmsbit(cidr);

  return true;
}

bool NetMask::ScanName(const char *buf, size_t len, std::string &name)
{
  const char *q = buf;
  const char *p = buf + len;
  while ((q < p) && isspace(*q))
    ++q;
  while ((p > q) && isspace(p[-1]))
    --p;
  size_t n = p - q;
  name = std::string(q, n);
  return (n != 0) ? true : false;
}


std::ostream &operator<<(std::ostream &os, const struct sockaddr &addr)
{
  char sa[256];
  char sp[16];

  switch(addr.sa_family)
  {
  case AF_INET:
    return os
      << ::inet_ntop(AF_INET, &((const struct sockaddr_in &) addr).sin_addr, sa, sizeof(sa))
      << ":"
      << (::sprintf(sp, "%u", ntohs(((const struct sockaddr_in &) addr).sin_port)), sp);
  case AF_INET6:
    return os
      << "["
      << ::inet_ntop(AF_INET6, &((const struct sockaddr_in6 &) addr).sin6_addr, sa, sizeof(sa))
      << "]"
      << (::sprintf(sp, ":%u", ntohs(((const struct sockaddr_in6 &) addr).sin6_port)), sp);
  case AF_LOCAL:
      return os << ((const struct sockaddr_un &) addr).sun_path;
  default:
    logexc << "unsupported sockaddr::sa_family = " << addr.sa_family << std::endl;
    return os;
  }
}

std::ostream &operator<<(std::ostream &os, const NetAddr &addr)
{
  char sa[256];

  switch(addr.domain())
  {
  case AF_INET:
    return os
      << ::inet_ntop(AF_INET, &addr.in().sin_addr, sa, sizeof(sa));
  case AF_INET6:
    return os
      << ::inet_ntop(AF_INET6, &addr.in6().sin6_addr, sa, sizeof(sa));
  default:
    logexc << "unsupported NetAddr::domain() = " << addr.domain() << std::endl;
    return os;
  }
}

std::ostream &operator<<(std::ostream &os, const NetMask &netmsk)
{
  const std::string &name = netmsk.name();
  if (!name.empty())
    return os << name;

  NetAddrData mask = netmsk.addr(0);
  mask ^= netmsk.addr(1);
  unsigned cidr = (~mask).getmsbit();
  if ((mask & netmsk.addr(0)).iszero() && ((mask & netmsk.addr(1)) == mask))
    return os << netmsk.addr(0) << "/" << cidr;

  return os << netmsk.addr(0) << "-" << netmsk.addr(1);
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


std::list<SockAddrData> &SockFN::AddrList(std::list<SockAddrData> &lst, const char *addr, const char *dfltsvc /* = nullptr */)
{
  const char *srvc;
  if ((strchr(addr, '/') != nullptr) || (((srvc = strrchr(addr, ':')) == nullptr) && (dfltsvc == nullptr))) {
    struct sockaddr_un saddr;
    if (strlen(addr) >= sizeof(saddr.sun_path))
      logexc << "failed to create SockAddr list item from \"" << addr << "\", pathname too long" << std::endl;
    saddr.sun_family = AF_LOCAL;
    strcpy(saddr.sun_path, addr);
    lst.push_back((SockAddrData &) SockAddrData().operator=((struct sockaddr &) saddr));
    return lst;
  }

  std::string straddr(addr, (srvc != nullptr) ? srvc : (const char *) (addr + strlen(addr)));
  if (srvc != nullptr) {
    ++srvc;
    if ((straddr.size() != 0) && (straddr.front() == '[') && (straddr.back() == ']')) {
      straddr.erase(straddr.begin());
      straddr.pop_back();
    } else if (strchr(straddr.c_str(), ':') != nullptr) {
      if (dfltsvc != nullptr) {
        straddr = addr;
        srvc = dfltsvc;
      } else {
        logexc << "failed to create SockAddr list item from \"" << addr << "\", malformed address" << std::endl;
      }
    }
  } else {
    srvc = dfltsvc;
  }
  struct addrinfo *aires, hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_ADDRCONFIG | AI_ALL;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  errno = 0;
  int airt = ::getaddrinfo(straddr.c_str(), srvc, &hints, &aires);
  switch (airt)
  {
  default:
    if (errno == 0)
      errno = ENOENT;
    logsxc << "getaddrinfo(addr = " << straddr << ", srvc = " << srvc <<") failed: " << gai_strerror(airt) << std::endl;
  case EAI_NODATA:
    aires = nullptr;
  case 0:
    break;
  }

  try { for (struct addrinfo *ai = aires; (ai != nullptr); ai = ai->ai_next) switch(ai->ai_family) {
  case AF_INET:
  case AF_INET6:
    lst.push_back((SockAddrData &) SockAddrData().operator=(*ai->ai_addr));
  } } catch(...) {
    if (aires != nullptr)
      ::freeaddrinfo(aires);
    throw;
  }
  if (aires != nullptr)
    ::freeaddrinfo(aires);

  return lst;
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

FD SockFN::Connect(const char *addr, const char *dfltsvc /* = nullptr */)
{
  const char *srvc;
  if ((strchr(addr, '/') != nullptr) || (((srvc = strrchr(addr, ':')) == nullptr) && (dfltsvc == nullptr))) {
    struct sockaddr_un saddr;
    if (strlen(addr) >= sizeof(saddr.sun_path))
      logexc << "failed to connect \"" << addr << "\", pathname too long" << std::endl;
    saddr.sun_family = AF_LOCAL;
    strcpy(saddr.sun_path, addr);
    return Connect((struct sockaddr *) &saddr);
  }

  std::string straddr(addr, (srvc != nullptr) ? srvc : (const char *) (addr + strlen(addr)));
  if (srvc != nullptr) {
    ++srvc;
    if ((straddr.size() != 0) && (straddr.front() == '[') && (straddr.back() == ']')) {
      straddr.erase(straddr.begin());
      straddr.pop_back();
    } else if (strchr(straddr.c_str(), ':') != nullptr) {
      if (dfltsvc != nullptr) {
        straddr = addr;
        srvc = dfltsvc;
      } else {
        logexc << "SockFN::Connect(\"" << addr << "\") failed, malformed address" << std::endl;
      }
    }
  } else {
    srvc = dfltsvc;
  }
  struct addrinfo *aires, hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_ADDRCONFIG | AI_ALL;
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

FD SockFN::Listen(const char *addr, const char *dfltsvc /* = nullptr */)
{
  const char *srvc;
  if ((strchr(addr, '/') != nullptr) || (((srvc = strrchr(addr, ':')) == nullptr) && (dfltsvc == nullptr))) {
    struct sockaddr_un saddr;
    if (strlen(addr) >= sizeof(saddr.sun_path))
      logexc << "failed to listen on \"" << addr << "\", pathname too long" << std::endl;
    saddr.sun_family = AF_LOCAL;
    strcpy(saddr.sun_path, addr);
    return Listen((struct sockaddr *) &saddr);
  }

  std::string straddr(addr, (srvc != nullptr) ? srvc : (const char *) (addr + strlen(addr)));
  if (srvc != nullptr) {
    ++srvc;
    if ((straddr.size() != 0) && (straddr.front() == '[') && (straddr.back() == ']')) {
      straddr.erase(straddr.begin());
      straddr.pop_back();
    } else if (strchr(straddr.c_str(), ':') != nullptr) {
      if (dfltsvc != nullptr) {
        straddr = addr;
        srvc = dfltsvc;
      } else {
        logexc << "SockFN::Listen(\"" << addr << "\") failed, malformed address" << std::endl;
      }
    }
  } else {
    srvc = dfltsvc;
  }
  struct addrinfo *aires, hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_ADDRCONFIG | AI_ALL;
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
  for (struct addrinfo *ai = aires; ai != nullptr; ai = ai->ai_next) switch (ai->ai_family) { // AF_INET takes precedence
  case AF_INET:
    try {
      sfd = Listen(ai->ai_addr);
    } catch(...) {
      ::freeaddrinfo(aires);
      throw;
    }
    ::freeaddrinfo(aires);
    return sfd;
  }
  try {
    sfd = Listen(aires->ai_addr);
  } catch(...) {
    ::freeaddrinfo(aires);
    throw;
  }
  ::freeaddrinfo(aires);
  return sfd;
}
