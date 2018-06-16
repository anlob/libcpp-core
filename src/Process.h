/** \file Process.h
 * Process handling stuff.
 *
 * \author Andreas Lobbes, andreas.lobbes@gmail.com
 * \date 2016-2018
 * \copyright GPLV3 and above + GCC RTL Exception 3.1
 * \see LICENSES at top level directory
 * \see http://www.gnu.org/licenses
 */
#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <map>
#include <string>


class Process
{
protected:
  Process(pid_t pid): pid_(pid), waited_(false), xstat_(0) {}
public:
  static Process *New(const char *path, const std::vector<std::string> &args, const std::map<std::string, std::string> *env = nullptr, const int (*stdfd)[3] = nullptr);
  static Process *New(int (*main)(const std::vector<std::string> &args), const std::vector<std::string> &args, const std::map<std::string, std::string> *env = nullptr, const int (*stdfd)[3] = nullptr);
  virtual ~Process();
  void Kill(int sig);
  bool Terminated();
  int Wait();

protected:
  pid_t pid_;
  bool waited_;
  int xstat_;
};


#endif // PROCESS_H_INCLUDED
