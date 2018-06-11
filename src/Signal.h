/** \file Signal.h
 * Signal handling stuff.
 *
 * \author Andreas Lobbes, andreas.lobbes@gmail.com
 * \date 2016-2018
 * \copyright GPLV3 and above + GCC RTL Exception 3.1
 * \see http://www.gnu.org/licenses
 */
#ifndef SIGNAL_H_INCLUDED
#define SIGNAL_H_INCLUDED

#include <csignal>


class SigSet
{
public:
  SigSet(sigset_t &data): sigset_(data) {}
  virtual ~SigSet() {}

  sigset_t &data() const { return sigset_; }

  SigSet &reset() { ::sigemptyset(&sigset_); return *this; }
  SigSet &setall() { ::sigfillset(&sigset_); return *this; }
  SigSet &set(int sig) { sigaddset(&sigset_, sig); return *this; }
  SigSet &clr(int sig) { sigdelset(&sigset_, sig); return *this; }

protected:
  sigset_t &sigset_;
};

class SigSetData: public SigSet
{
public:
  SigSetData(): SigSet(sigset_) { reset(); }
  virtual ~SigSetData() {}

protected:
  sigset_t sigset_;
};

class SigMask
{
public:
  SigMask(const SigSet &mask);
  virtual ~SigMask();
protected:
  sigset_t oldset_;
};


#endif // SIGNAL_H_INCLUDED
