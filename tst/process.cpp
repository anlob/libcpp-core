#include <cstring>
#include <memory>
#include <stdexcept>
#include <system_error>
#include "process.h"
#include "../src/FD.h"
#include "../src/FDStream.h"
#include "../src/Log.h"
#include "../src/Process.h"

using namespace std;


#if (_TEST_ALL == 1) || (_TEST_GRP_PROC == 1) || (_TEST_PROC_EXEC == 1)
void test_proc_exec()
{
  PFD pfd;
  int stdfd[3] = {-1, pfd.out, -1};
  std::vector<string> args;
  args.push_back("echo");
  args.push_back("TESTDATA");
  unique_ptr<Process> p(Process::New("echo", args, nullptr, &stdfd));
  pfd.out.Close();
  FDStream fs(pfd.in.Detach(), ios::in);
  char buf[16];
  if (!fs.read(buf, 8))
    logsxc << "fs.read() failed" << endl;
  if (memcmp(buf, "TESTDATA", 8) != 0)
    logexc << "failed to read TESTDATA from redirected stdout" << endl;
  p->Wait();
  p.reset(nullptr);
}
#endif

#if (_TEST_ALL == 1) || (_TEST_GRP_PROC == 1) || (_TEST_PROC_FN == 1)
static int test_proc_main(const vector<string> &args)
{
  return 123;
}
void test_proc_fn()
{
  std::vector<string> args;
  unique_ptr<Process> p(Process::New(test_proc_main, args));
  int stat = p->Wait();
  if (!WIFEXITED(stat) || (WEXITSTATUS(stat) != 123))
    logexc << "test_proc_main terminated with unexpected exit status" << endl;
  p.reset(nullptr);
}
#endif
