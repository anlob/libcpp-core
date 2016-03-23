#include <cerrno>
#include <stdexcept>
#include <system_error>
#include <sstream>
#include "../src/Log.h"
#include "log.h"

using namespace std;


#if (_TEST_ALL == 1) || (_TEST_GRP_LOG == 1) || (_TEST_LOG_ERR == 1)
void test_log_err()
{
  stringstream sfd;
  ostream *oldlog = Log::set_device(&sfd);
  logerr << "LogErrTest" << endl;
  Log::set_device(oldlog);
  if (sfd.str().find("LogErrTest") == string::npos)
    logexc << "LogErrTest not found in log data" << endl;
}
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_LOG == 1) || (_TEST_LOG_EXC == 1)
void test_log_exc()
{
  stringstream sfd;
  ostream *oldlog = Log::set_device(&sfd);
  string xcmsg;
  bool xcaught = false;
  try {
    logexc << "LogExcTest" << endl;
  } catch(exception &e) {
    xcmsg = e.what();
    xcaught = true;
  } catch(...) {
  }
  Log::set_device(oldlog);
  if (!xcaught)
    logexc << "exception log string not caught" << endl;
  if (xcmsg.find("LogExcTest") == string::npos)
    logexc << "LogExcTest not found in exception log string" << endl;
  if (sfd.str().find("LogExcTest") == string::npos)
    logexc << "LogExcTest not found in log data" << endl;
}
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_LOG == 1) || (_TEST_LOG_SXC == 1)
void test_log_sxc()
{
  stringstream sfd;
  ostream *oldlog = Log::set_device(&sfd);
  string xcmsg;
  int err = -1;
  bool xcaught = false;
  try {
    errno = EPIPE;
    logsxc << "LogSxcTest" << endl;
  } catch(system_error &e) {
    xcmsg = e.what();
    err = e.code().value();
    xcaught = true;
  } catch(...) {
  }
  Log::set_device(oldlog);
  if (!xcaught)
    logexc << "exception log string not caught" << endl;
  if (xcmsg.find("LogSxcTest") == string::npos)
    logexc << "LogSxcTest not found in exception log string" << endl;
  if (err != EPIPE)
    logexc << "different error code returned in system_error exception" << endl;
  if (sfd.str().find("LogSxcTest") == string::npos)
    logexc << "LogSxcTest not found in log data" << endl;
}
#endif
