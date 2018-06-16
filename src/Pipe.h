/** \file Pipe.h
 * Pipe stuff.
 *
 * \author Andreas Lobbes, andreas.lobbes@gmail.com
 * \date 2016-2018
 * \copyright GPLV3 and above + GCC RTL Exception 3.1
 * \see LICENSES at top level directory
 * \see http://www.gnu.org/licenses
 */
#ifndef PIPE_H_INCLUDED
#define PIPE_H_INCLUDED

#include <cerrno>
#include <system_error>
#include <istream>
#include <ostream>
#include <iostream>
#include <iosfwd>
#include "Log.h"
#include "FD.h"
#include "FDStream.h"


template<typename _E, typename _Tr = std::char_traits<_E> >
class BasicPipe: public PFD, public BasicFDStream<_E, _Tr>
{
public:
  BasicPipe(std::ios::openmode mode): PFD(), BasicFDStream<_E, _Tr>(Detach(mode), mode) {}
  virtual ~BasicPipe() {}

private:
  int Detach(std::ios::openmode mode)
  {
    switch(mode)
    {
    case std::ios::in:
      return in.Detach();
    case std::ios::out:
      return out.Detach();
    default:
      logerr << "bad openmode " << mode << " given" << std::endl;
      throw std::system_error(std::error_code(errno = EINVAL, std::generic_category()));
    }
  }
};

typedef BasicPipe<char> Pipe;
typedef BasicPipe<wchar_t> WPipe;


#endif // PIPE_H_INCLUDED
