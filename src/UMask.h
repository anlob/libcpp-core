#ifndef UMASK_H_INCLUDED
#define UMASK_H_INCLUDED

#include <sys/types.h>
#include <sys/stat.h>


class UMask
{
public:
  UMask(mode_t mode) { oldmode_ = ::umask(mode); }
  virtual ~UMask() { ::umask(oldmode_); }
protected:
  mode_t oldmode_;
};


#endif // UMASK_H_INCLUDED
