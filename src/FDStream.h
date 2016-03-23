#ifndef FDSTREAM_H_INCLUDED
#define FDSTREAM_H_INCLUDED

#include <ext/stdio_filebuf.h>
#include <iostream>
#include <fstream>
#include <iosfwd>
#include <string>
#include <cstdio>


template<typename _E, typename _Tr = std::char_traits<_E> >
class BasicFDStream: protected __gnu_cxx::stdio_filebuf<_E, _Tr>, public std::basic_iostream<_E, _Tr>
{
public:

  BasicFDStream(int fd, std::ios::openmode mode = std::ios::in | std::ios::out):
    __gnu_cxx::stdio_filebuf<_E, _Tr>(fd, mode),
    std::basic_iostream<_E, _Tr>(static_cast<__gnu_cxx::stdio_filebuf<_E, _Tr> *>(this))
  {
  }

  virtual ~BasicFDStream()
  {
  }

  int fd() { return __gnu_cxx::stdio_filebuf<_E, _Tr>::fd(); }
};

typedef BasicFDStream<char> FDStream;
typedef BasicFDStream<wchar_t> WFDStream;


#endif // FDSTREAM_H_INCLUDED
