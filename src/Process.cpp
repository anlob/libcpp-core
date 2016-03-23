#include <cstdlib>
#include <cerrno>
#include <system_error>
#include <fcntl.h>
#include "Log.h"
#include "Process.h"

using namespace std;


Process *Process::New(const char *path, const vector<string> &args, const map<string, string> *env /* = nullptr */, const int (*stdfd)[3] /* = nullptr */)
{
  pid_t pid = ::fork();
  if (pid == 1)
    logsxc << "fork() failed" << endl;
  if (pid != 0)
    return new Process(pid);

  const char **argv = (const char **) new char*[(args.size() + 1) * sizeof(char *)];
  for (unsigned i = 0; i < args.size(); ++i)
    argv[i] = args[i].c_str();
  argv[args.size()] = nullptr;

  if (env != nullptr) for (std::map<string, string>::const_iterator it = env->begin();  it != env->end(); ++it) {
    if (setenv(it->first.c_str(), it->second.c_str(), 1) == -1) {
      logerr << "setenv() failed with " << errno << endl;
      abort();
    }
  }

  if (stdfd != nullptr) for (unsigned i = 0; i < 3; ++i) {
    int fd = (*stdfd)[i];
    if (fd == -1)
      continue;
    if (::dup2(fd, i) == -1) {
      logerr << "dup2(old = " << fd << ", new = " << i << ") failed with " << errno << endl;
      abort();
    }
    ::close(fd);
  }

  execvp(path, (char **) argv);
  logerr << "execvp(path = " << path << ") failed with " << errno << endl;
  abort();
  return nullptr;
}

Process *Process::New(int (*main)(const std::vector<std::string> &args), const std::vector<std::string> &args, const std::map<std::string, std::string> *env /* = nullptr */, const int (*stdfd)[3] /* = nullptr */)
{
  pid_t pid = ::fork();
  if (pid == 1)
    logsxc << "fork() failed" << endl;
  if (pid != 0)
    return new Process(pid);

  if (env != nullptr) for (std::map<string, string>::const_iterator it = env->begin();  it != env->end(); ++it) {
    if (setenv(it->first.c_str(), it->second.c_str(), 1) == -1) {
      logerr << "setenv() failed with " << errno << endl;
      abort();
    }
  }

  if (stdfd != nullptr) for (unsigned i = 0; i < 3; ++i) {
    int fd = (*stdfd)[i];
    if (fd == -1)
      continue;
    if (::dup2(fd, i) == -1) {
      logerr << "dup2(old = " << fd << ", new = " << i << ") failed with " << errno << endl;
      abort();
    }
    ::close(fd);
  }

  exit(main(args));
  return nullptr;
}

Process::~Process()
{
  Kill(SIGTERM);
  Wait();
}

void Process::Kill(int sig)
{
  if (waited_)
    return;

  if (::kill(pid_, sig) == -1)
    logwrn << "kill(pid = " << pid_ << ", sig = " << sig << ") failed with " << errno << endl;
}

bool Process::Terminated()
{
  if (waited_)
    return true;

  switch (::waitpid(pid_, &xstat_, WNOHANG)) {
  case -1:
    logsxc << "waitpid(pid = " << pid_ << ") failed" << endl;
  case 0:
    return false;
  default:
    waited_ = true;
    return xstat_;
  }
}

int Process::Wait()
{
  if (waited_)
    return xstat_;

  while (::waitpid(pid_, &xstat_, 0) == -1) {
    if (errno == EINTR)
      continue;
    logsxc << "waitpid(pid = " << pid_ << ") failed" << endl;
  }

  waited_ = true;
  return xstat_;
}
