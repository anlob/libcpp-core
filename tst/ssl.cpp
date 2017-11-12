#include <stdexcept>
#include "../src/Log.h"
#include "../src/FD.h"
#include "../src/Sock.h"
#include "../src/SSL.h"
#include "ssl.h"

using namespace std;


#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_CONN == 1)
void test_ssl_conn()
{
  FD sfd = SockFN::Connect("web.de:443");
  SSLH sslh((int) sfd);
  sfd.Detach();
  if (!sslh.connect())
    logexc << "ssl connect test failed" << endl;
}
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_SHUT == 1)
void test_ssl_shut()
{

}
#endif
