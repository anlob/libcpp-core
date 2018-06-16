/** \file FD.h
 * File descriptor stuff.
 *
 * \author Andreas Lobbes, andreas.lobbes@gmail.com
 * \date 2016-2018
 * \copyright GPLV3 and above + GCC RTL Exception 3.1
 * \see LICENSES at top level directory
 * \see http://www.gnu.org/licenses
 */
#ifndef FD_H_INCLUDED
#define FD_H_INCLUDED

#include <unistd.h>


class FD
{
public:

  FD(FD &&from): fd_(from.Detach()) {}
  FD(const FD &from) { *this = from; }
  FD(int fd = -1): fd_(fd) {}
  virtual ~FD() { Close(); }

  void Close();
  int Detach() { int fd = fd_; fd_ = -1; return fd; }

  operator int() const { return fd_; }
  FD &operator=(FD &&from) { fd_ = from.Detach(); return *this; }
  FD &operator=(const FD &from);
  FD &operator=(int fd) { Close(); fd_ = fd; return *this; }

protected:
  int fd_;
};

struct PFD
{
  PFD();
  virtual ~PFD() {}

  FD in;
  FD out;
};


#endif // FD_H_INCLUDED
