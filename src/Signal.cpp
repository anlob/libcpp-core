#include "Log.h"
#include "Signal.h"

using namespace std;


SigMask::SigMask(const SigSet &mask)
{
  if (::sigprocmask(SIG_BLOCK, &mask.data(), &oldset_) < 0)
    logsxc << "sigprocmask() failed" << endl;
}

SigMask::~SigMask()
{
  ::sigprocmask(SIG_SETMASK, &oldset_, nullptr);
}
