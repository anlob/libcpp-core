#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <istream>
#include <ostream>
#include <iostream>
#include <iosfwd>
#include "../src/Log.h"
#include "../src/Pipe.h"
#include "pipe.h"

using namespace std;


#if (_TEST_ALL == 1) || (_TEST_GRP_PIPE == 1) || (_TEST_PIPE_IO == 1)
void test_pipe_io()
{
  Pipe p(ios::in);

  write(p.PFD::out, "TEST", 4); // just 4 bytes, we don't want to get self-blocked!
  char buf[4];
  p.read(buf, 4);
  if (memcmp(buf, "TEST", 4) != 0)
    logexc << "primitve pipe io test failed" << endl;
}
#endif
