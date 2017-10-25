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
  class MySink: public OStreamBuf
  {
    char data_[16];
    size_t size_;

    int flush()
    {
      for (char *q = pbase(); q < pptr();++q) {
        if (size_ == sizeof(data_))
          logexc << "unexpected overflow in primitive OStreamBuf data test" << endl;
        data_[size_++] = *q;
      }
      return 0;
    }

  public:
    MySink(): size_(0) {}
    const char *data() const { return data_; }
    size_t size() const { return size_; }
  };

  MySink *sink = new MySink;
  ostream os(sink);
  os << "hey" << endl;
  if ((sink->size() != 4) || (memcmp(sink->data(), "hey\n", 4) != 0))
    logexc << "primitive OStreamBuf data test failed" << endl;
}
#endif
