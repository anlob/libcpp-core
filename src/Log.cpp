/** \file Log.cpp
 * Logging stuff.
 *
 * \author Andreas Lobbes, andreas.lobbes@gmail.com
 * \date 2016-2018
 * \copyright GPLV3 and above + GCC RTL Exception 3.1
 * \see http://www.gnu.org/licenses
 */
#include <exception>
#include <system_error>
#include <string>
#include <cerrno>
#include "Log.h"

using namespace std;


static ostream *Log_device_ = &clog;

static void Log_cleanup() __attribute__((destructor(0)));
static void Log_cleanup()
{
  Log::close_device(Log_device_);
  Log_device_ = &clog;
}


ostream &Log::get_device()
{
  return *Log_device_;
}

ostream *Log::set_device(ostream *os)
{
  ostream *dv = Log_device_;
  Log_device_ = os;
  return dv;
}

void Log::close_device(ostream *os)
{
  if ((os != &clog) && (os != &cerr) && (os != &cout))
    delete os;
}

Log::xcstream &Log::xcstream::operator<<(ostream &(*__mf)(ostream &))
{
  ostream &(*__cmp)(ostream &) = endl;
  if (__mf != __cmp) {
    __mf(*this);
  } else {
    string msg = str();
    if (Log::error_enabled(pf_.c_str()))
      Log::get_device() << pf_ << ": errno = " << err_ << ", " << msg << endl;
    if (!~err_)
      throw runtime_error(msg);
    else
      throw system_error(error_code(err_, system_category()), msg);
  }

  return *this;
}

bool Log::trace_enabled(const char *fn)
{
  return true;
}

bool Log::info_enabled(const char *fn)
{
  return true;
}

bool Log::warning_enabled(const char *fn)
{
  return true;
}

bool Log::error_enabled(const char *fn)
{
  return true;
}
