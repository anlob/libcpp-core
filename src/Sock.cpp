#include <stdexcept>
#include <system_error>
#include <unistd.h>
#include <cerrno>
#include <ctime>
#include <cstring>
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
