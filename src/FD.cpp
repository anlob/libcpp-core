#include <cerrno>
#include <system_error>
#include <unistd.h>
#include "Log.h"
#include "FD.h"

using namespace std;


void FD::Close()
{
  if (fd_ == -1)
    return;

  if (::close(fd_) == -1)
    logsxc << "close() failed" << endl;
  fd_ = -1;
}

PFD::PFD()
{
  int pfd[2];
  if (::pipe(pfd) == -1)
    logsxc << "pipe() failed" << endl;
  in = pfd[0];
  out = pfd[1];
}
