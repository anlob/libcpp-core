#include <unistd.h>
#include <istream>
#include <ostream>
#include <iostream>
#include "../src/App.h"
#include "app.h"

using namespace std;


#if (_TEST_ALL == 1) || (_TEST_GRP_APP == 1) || (_TEST_APP_CONFDIR == 1)
static class TestAppCleanup
{
public:
  TestAppCleanup()
  {
    pid_ = ::getpid();
  }
  ~TestAppCleanup()
  {
    if (pid_ != ::getpid())
      return;
    string d = App::get_confdir();
    ::rmdir(d.c_str());
  }
private:
  pid_t pid_;
} testAppCleanup_;
#endif


#if (_TEST_ALL == 1) || (_TEST_GRP_APP == 1) || (_TEST_APP_HOMEDIR == 1)
void test_app_homedir()
{
  App::get_homedir();
}
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_APP == 1) || (_TEST_APP_CONFDIR == 1)
void test_app_confdir()
{
  App::get_confdir();
}
#endif
