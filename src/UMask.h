/** \file UMask.h
 * ::umask() stuff.
 *
 * \author Andreas Lobbes, andreas.lobbes@gmail.com
 * \date 2016-2018
 * \copyright GPLV3 and above + GCC RTL Exception 3.1
 * \see LICENSES at top level directory
 * \see http://www.gnu.org/licenses
 */
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
