/** \file Log.h
 * Logging stuff.
 *
 * \author Andreas Lobbes, andreas.lobbes@gmail.com
 * \date 2016-2018
 * \copyright GPLV3 and above + GCC RTL Exception 3.1
 * \see http://www.gnu.org/licenses
 */
#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#include <ostream>
#include <iostream>
#include <sstream>
#include <iosfwd>
#include <cerrno>


struct Log
{
  static std::ostream &get_device();
  static std::ostream *set_device(std::ostream *os);
  static void close_device(std::ostream *os);

  class xcstream: public std::ostringstream {
  public:
    xcstream(const char *pf, int err = -1): pf_(pf), err_(err) {}
    template<typename _T> xcstream &operator<<(const _T &v) { (static_cast<std::ostringstream&>(*this)) << v; return *this; }
    xcstream &operator<<(std::ostream &(*__mf)(std::ostream &));
  protected:
    std::string pf_;
    int err_;
  };

  static bool trace_enabled(const char *fn);
  static bool info_enabled(const char *fn);
  static bool warning_enabled(const char *fn);
  static bool error_enabled(const char *fn);
};


#define logtrc  if (Log::trace_enabled(__PRETTY_FUNCTION__)) Log::get_device() <<  __PRETTY_FUNCTION__ << ": "
#define loginf  if (Log::info_enabled(__PRETTY_FUNCTION__)) Log::get_device() <<  __PRETTY_FUNCTION__ << ": "
#define logwrn  if (Log::warning_enabled(__PRETTY_FUNCTION__)) Log::get_device() <<  __PRETTY_FUNCTION__ << ": "
#define logerr  if (Log::error_enabled(__PRETTY_FUNCTION__)) Log::get_device() <<  __PRETTY_FUNCTION__ << ": "
#define logexc  Log::xcstream(__PRETTY_FUNCTION__)
#define logsxc  Log::xcstream(__PRETTY_FUNCTION__, errno)


#endif // LOG_H_INCLUDED
