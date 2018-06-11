/** \file App.cpp
 * Application stuff.
 *
 * \author Andreas Lobbes, andreas.lobbes@gmail.com
 * \date 2016-2018
 * \copyright GPLV3 and above + GCC RTL Exception 3.1
 * \see http://www.gnu.org/licenses
 */
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <cerrno>
#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include <sys/stat.h>
#include <sys/types.h>
#include "Log.h"
#include "App.h"

using namespace std;


static string *App_name_ = nullptr;
static string *App_homedir_ = nullptr;
static string *App_confdir_ = nullptr;

static void App_cleanup() __attribute__((destructor(0)));
static void App_cleanup()
{
  delete App_name_;
  App_name_ = nullptr;
  delete App_homedir_;
  App_homedir_ = nullptr;
  delete App_confdir_;
  App_confdir_ = nullptr;
}


const string &App::get_name()
{
  if (App_name_ != nullptr)
    return *App_name_;

  stringstream sfd;
  sfd << "/proc/" << ::getpid() << "/cmdline";
  fstream f(sfd.str().c_str(), ios::in);
  if (!f.is_open())
    logsxc << "open \"" << sfd.str() << "\" failed" << endl;
  App_name_ = new string;
  char c;
  while (f.get(c) && (c != '\0')) {
    if (c != '/')
      App_name_->append(1, c);
    else
      App_name_->clear();
  }
  return *App_name_;
}

const string &App::get_homedir()
{
  if (App_homedir_ != nullptr)
    return *App_homedir_;

  const char *q = getenv("HOME");
  if (q == nullptr)
    logexc << "HOME environment not set" << endl;
  string s = q;
  string::iterator it;
  if (((it = s.begin()) == s.end()) || (*it != '/'))
    logexc << "HOME environment is not an absolute path" << endl;
  string::iterator itx;
  if (((itx = --s.end()) != it) && (*itx == '/'))
    s.erase(itx);

  return *(App_homedir_ = new string(s));
}

const string &App::get_confdir()
{
  if (App_confdir_ != nullptr)
    return *App_confdir_;

  string s = get_homedir();
  if ((*--s.end()) != '/')
    s += "/";
  s += ".config";
  ::mkdir(s.c_str(), 0700);
  s += "/";
  s += get_name();
  ::mkdir(s.c_str(), 0700);

  struct stat st;
  if ((::stat(s.c_str(), &st)) || !S_ISDIR(st.st_mode))
    logexc << "failed to create config directory \"" << s << "\"" << endl;

  return *(App_confdir_ = new string(s));
}
