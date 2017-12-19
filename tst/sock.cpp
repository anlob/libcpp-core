#include <iostream>
#include <istream>
#include <ostream>
#include <iosfwd>
#include <string>
#include <cstring>
#include <stdexcept>
#include "../src/Log.h"
#include "../src/Sock.h"
#include "sock.h"

using namespace std;


#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_IPV6CONN == 1)
void test_sock_ipv6conn()
{
  SockFN::Connect("[::1]:ssh");
}
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_HTTPREQ == 1)
void test_sock_httpreq()
{
#if 0
  Sock sock(SockFN::Connect("web.de:80"));  // works too
#else
  Sock sock("web.de:80");
#endif

  sock << "GET / HTTP/1.0\r\nHost: web.de\r\n\r\n";
  sock.flush();
  sock.shutwr();

  string s;
  if (!getline(sock.in(), s) || (strncmp(s.c_str(), "HTTP/", 5) != 0))
    logexc << "bad HTTP response" << endl;
}
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_SHUT == 1)
void test_sock_shut()
{
  Sock sock;
  char c;
  sock.get(c);
  sock.put(c);
}
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_LISTEN == 1)
void test_sock_listen()
{
  SockFN::Listen("[::1]:7421");
}
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_ADDRSTR == 1)
void test_sock_addrstr()
{
  struct sockaddr_in sin;
  inet_pton(sin.sin_family = AF_INET, "192.168.0.1", &sin.sin_addr);
  sin.sin_port = htons(1234);
  string s = SockFN::AddrStr((struct sockaddr *) &sin);
  if (s != "192.168.0.1:1234")
    logexc << "SockFN::AddrStr() failed to convert 192.168.0.1:1234" << endl;

  struct sockaddr_in6 sin6;
  inet_pton(sin6.sin6_family = AF_INET6, "::1", &sin6.sin6_addr);
  sin6.sin6_port = htons(1234);
  s = SockFN::AddrStr((struct sockaddr *) &sin6);
  if (s != "[::1]:1234")
    logexc << "SockFN::AddrStr() failed to convert [::1]:1234" << endl;
}
#endif
