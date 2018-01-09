#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
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
  SockFN::Connect("::1", "ssh");
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

#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_ADDRLST == 1)
void test_sock_addrlst()
{
  std::list<SockAddrData> alst;

  alst.clear();
  if (!SockFN::AddrList(alst, "foo.bar", "*").empty())
    logexc << "SockFN::AddrList() failed to create empty list for \"foo.bar\"" << endl;
  alst.clear();
  if (SockFN::AddrList(alst, "localhost", "*").empty())
    logexc << "SockFN::AddrList() failed to create list for \"localhost\"" << endl;
  for (std::list<SockAddrData>::const_iterator it = alst.begin(); it != alst.end(); ++it) switch(it->domain())
  {
  case AF_INET:
    if (*it != (SockAddrData().svc("*") = "127.0.0.1"))
      logexc << "SockFN::AddrList() failed to create proper IPV4 list entry for \"localhost\"" << endl;
    break;
  case AF_INET6:
    if (*it != (SockAddrData().svc("*") = "::1"))
      logexc << "SockFN::AddrList() failed to create proper IPV6 list entry for \"localhost\"" << endl;
    break;
  }
}
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_NETADDR == 1)
void test_sock_netaddr()
{
  NetAddrData a1 = "192.168.77.0";
  NetAddrData a2 = "192.168.77.255";
  NetAddrData ar = ((const NetAddr &) a1);
  if (ar != NetAddrData("192.168.77.0"))
    logexc << "NetAddr::operator=(192.168.77.0) failed to assign" << endl;
  ar ^= ((const NetAddr &) a2);   // address range bits
  if (ar != NetAddrData("0.0.0.255"))
    logexc << "NetAddr::operator^=(192.168.77.0, 192.168.77.255) failed to compute" << endl;
  NetAddrData nm = ~ar;           // invers => netmask
  if (nm != NetAddrData("255.255.255.0"))
    logexc << "NetAddr::operator~(0.0.0.255) failed to compute" << endl;
  unsigned cidr = nm.getmsbit();  // CIDR netmask bits
  if (cidr != 24)
    logexc << "NetAddr::getmsbit(255.255.255.0) failed to compute" << endl;
  NetAddrData r1 = a1 & ar;
  if (r1 != NetAddrData("0.0.0.0"))
    logexc << "NetAddr::operator&(192.168.77.0, 0.0.0.255) failed to compute" << endl;
  NetAddrData r2 = a2 & ar;
  if (r2 != NetAddrData("0.0.0.255"))
    logexc << "NetAddr::operator&(192.168.77.255, 0.0.0.255) failed to compute" << endl;
  if (!r1.iszero())
    logexc << "NetAddr::iszero(0.0.0.0) failed to compute" << endl;
  if (r2.iszero())
    logexc << "NetAddr::iszero(0.0.0.255) failed to compute" << endl;
}
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_NETMASK == 1)
void test_sock_netmask()
{
  NetMask nmask;

  nmask = "192.168.77.0/24";
  if (!nmask.valid())
    logexc << "NetMask not valid after assignment (192.168.77.0/24)" << endl;
  if (!nmask.match(NetAddrData("192.168.77.255")))
    logexc << "NetMask did not match 192.168.77.255" << endl;
  if (!nmask.match(NetAddrData("192.168.77.0")))
    logexc << "NetMask did not match 192.168.77.0" << endl;
  if (nmask.match(NetAddrData("192.168.78.0")))
    logexc << "NetMask erroneously matched 192.168.78.0" << endl;
  if (nmask.match(NetAddrData("192.168.76.255")))
    logexc << "NetMask erroneously matched 192.168.76.255" << endl;
  stringstream sfd;
  sfd << nmask;
  if (sfd.str() != "192.168.77.0/24")
    logexc << "wrong output of NetMask stream operator for 192.168.77.0/24" << endl;

  nmask = "fd00::/61";
  if (!nmask.valid())
    logexc << "NetMask not valid after assignment (fd00::/61)" << endl;
  if (!nmask.match(NetAddrData("fd00::21cb:3dad:2e8c:3027")))
    logexc << "NetMask did not match fd00::21cb:3dad:2e8c:3027" << endl;
  sfd.clear();
  sfd.seekp(0);
  sfd.seekg(0);
  sfd.str(string());
  sfd << nmask;
  if (sfd.str() != "fd00::/61")
    logexc << "wrong output of NetMask stream operator for fd00::/61" << endl;
}
#endif
