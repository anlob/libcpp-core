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


#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_HTTPREQ == 1)
void test_sock_httpreq()
{
  Sock sock("web.de:80");

  sock << "GET / HTTP/1.0\nHost: web.de\n" << endl;
  sock.shutwr();

  string s;
  if (!getline(sock.in(), s) || (strncmp(s.c_str(), "HTTP/", 5) != 0))
    logexc << "bad HTTP response" << endl;
}
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_SHUT == 1)
void test_sock_shut()
{
  Sock sock(nullptr, nullptr);
  char c;
  sock.get(c);
  sock.put(c);
}
#endif
