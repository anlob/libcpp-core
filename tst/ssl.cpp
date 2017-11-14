#include <string>
#include <cstring>
#include <stdexcept>
#include "../src/Log.h"
#include "../src/FD.h"
#include "../src/Sock.h"
#include "../src/SSL.h"
#include "ssl.h"

using namespace std;


static SSLH do_ssl_conn()
{
  FD sfd = SockFN::Connect("web.de:443");
  SSLH sslh((int) sfd);
  sfd.Detach();
  if (!sslh.connect())
    logexc << "ssl connect test failed" << endl;
  return sslh;
}

#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_CONN == 1)
void test_ssl_conn()
{
  do_ssl_conn();
}
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_SHUT == 1)
void test_ssl_shut()
{
  SSLH sslh = do_ssl_conn();
  if (!sslh.shutdown())
    logexc << "ssl shutdown test failed" << endl;
}
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_VIFY == 1)
void test_ssl_vify()
{
  SSLH sslh = do_ssl_conn();
  if (!sslh.verify())
    logexc << "ssl verify test failed" << endl;
}
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_RECONN == 1)
void test_ssl_reconn()
{
  SSLH sslh = do_ssl_conn();
  if (!sslh.shutdown())
    logexc << "ssl shutdown failed in reconnect test" << endl;
  if (!sslh.connect())
    logexc << "ssl reconnect test failed" << endl;
}
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_HTTPREQ == 1)
void test_ssl_httpreq()
{
  SSLH sslh = do_ssl_conn();
  const char *rq = "GET / HTTP/1.1\r\nHost: web.de\r\nConnection: close\r\n\r\n";
  if (sslh.write(rq, strlen(rq)) < 0)
    logexc << "ssl write failed in http request test" << endl;
  char buf[512];
  int n, sz = 0;
  while (1) {
    n = sslh.read(buf + sz, sizeof(buf) - sz);
    if (n < 0)
      logexc << "ssl read failed in http request test" << endl;
    if (n == 0)
      logexc << "unexpected EOF detected in ssl read of http request test" << endl;
    char *p = (char *) memchr(buf + sz, '\n', n);
    if (p != nullptr) {
      *p = '\0';
      break;
    }
    if ((sz += n) == sizeof(buf))
      logexc << "unexpected buffer overflow in ssl read of http request test" << endl;
  }
  if (strncmp(buf, "HTTP/", 5) != 0)
    logexc << "bad HTTPS response" << endl;
}
#endif
