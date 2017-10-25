#include <istream>
#include <ostream>
#include <cstring>
#include "../src/Log.h"
#include "../src/StreamBuf.h"
#include "streambuf.h"

using namespace std;


#if (_TEST_ALL == 1) || (_TEST_GRP_STMBF == 1) || (_TEST_STMBF_INP == 1)
void test_stmbf_inp()
{
  class MyFeed: public IStreamBuf
  {
    bool filled_;

    int fill(size_t bufsz)
    {
      if (!filled_)
        return filled_ = true, strlen(strcpy(IStreamBuf::gptr(), "hey")) + 1;
      else
        return 0; // EOF
    }

  public:
    MyFeed(): filled_(false) {}
  };

  istream is(new MyFeed);
  for (int i = 0; i < 4; ++i) {
    if (is.get() != ((const char *) "hey")[i])
      logexc << "primitive IStreamBuf data test failed" << endl;
  }
  if (is.get() != -1)
    logexc << "primitive IStreamBuf EOF test failed" << endl;
}
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_STMBF == 1) || (_TEST_STMBF_OUT == 1)
void test_stmbf_out()
{
}
#endif
