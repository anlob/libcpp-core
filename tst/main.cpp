#include "conf.h"
#include "log.h"
#include "app.h"
#include "sock.h"
#include "pipe.h"
#include "process.h"
#include "iomanip.h"
#include "streambuf.h"
#include "ssl.h"

using namespace std;


int main()
{
#if (_TEST_ALL == 1) || (_TEST_GRP_LOG == 1) || (_TEST_LOG_ERR == 1)
  test_log_err();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_LOG == 1) || (_TEST_LOG_EXC == 1)
  test_log_exc();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_LOG == 1) || (_TEST_LOG_SXC == 1)
  test_log_sxc();
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_APP == 1) || (_TEST_APP_HOMEDIR == 1)
  test_app_homedir();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_APP == 1) || (_TEST_APP_CONFDIR == 1)
  test_app_confdir();
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_HTTPREQ == 1)
  test_sock_httpreq();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_SHUT == 1)
  test_sock_shut();
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_PIPE == 1) || (_TEST_PIPE_IO == 1)
  test_pipe_io();
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_PROC == 1) || (_TEST_PROC_EXEC == 1)
  test_proc_exec();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_PROC == 1) || (_TEST_PROC_FN == 1)
  test_proc_fn();
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_IOMNP == 1) || (_TEST_IOMNP_GETCSTR == 1)
  test_iomnp_getcstr();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_IOMNP == 1) || (_TEST_IOMNP_PUTCSTR == 1)
  test_iomnp_putcstr();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_IOMNP == 1) || (_TEST_IOMNP_GETCINT == 1)
  test_iomnp_getcint();
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_STMBF == 1) || (_TEST_STMBF_INP == 1)
  test_stmbf_inp();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_STMBF == 1) || (_TEST_STMBF_OUT == 1)
  test_stmbf_out();
#endif


#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_CONN == 1)
  test_ssl_conn();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_SHUT == 1)
  test_ssl_shut();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_VIFY == 1)
  test_ssl_vify();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_RECONN == 1)
  test_ssl_reconn();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_HTTPREQ == 1)
  test_ssl_httpreq();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_SOCK == 1)
  test_ssl_sock();
#endif

  return 0;
}
