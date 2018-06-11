/** \file FD.cpp
 * File descriptor stuff.
 *
 * \author Andreas Lobbes, andreas.lobbes@gmail.com
 * \date 2016-2018
 * \copyright GPLV3 and above + GCC RTL Exception 3.1
 * \see http://www.gnu.org/licenses
 */
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

FD &FD::operator=(const FD &from)
{
  Close();
  if ((from.fd_ != -1) && ((fd_ = ::dup(from.fd_)) == -1))
    logsxc << "dup(from.fd_) failed" << std::endl;
  return *this;
}

PFD::PFD()
{
  int pfd[2];
  if (::pipe(pfd) == -1)
    logsxc << "pipe() failed" << endl;
  in = pfd[0];
  out = pfd[1];
}
