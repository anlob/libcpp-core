/** \file Signal.cpp
 * Signal handling stuff.
 *
 * \author Andreas Lobbes, andreas.lobbes@gmail.com
 * \date 2016-2018
 * \copyright GPLV3 and above + GCC RTL Exception 3.1
 * \see http://www.gnu.org/licenses
 */
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
