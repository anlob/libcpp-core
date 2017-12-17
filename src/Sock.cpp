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


FD SockFN::Connect(struct sockaddr *addr)
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
    return sfd;
  }
}

FD SockFN::Connect(const char *addr)
{
  const char *srvc;
  if ((strchr(addr, '/') != nullptr) || ((srvc = strrchr(addr, ':')) == nullptr)) {
    struct sockaddr_un saddr;
    saddr.sun_family = AF_LOCAL;
    strncpy(saddr.sun_path, addr, sizeof(saddr.sun_path) - 1);
    saddr.sun_path[sizeof(saddr.sun_path) - 1] = '\0';
    return Connect((struct sockaddr *) &saddr);
  }

  std::string straddr(addr, srvc++);
  if ((straddr.size() != 0) && (straddr.front() == '[') && (straddr.back() == ']')) {
    straddr.erase(straddr.begin());
    straddr.pop_back();
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
