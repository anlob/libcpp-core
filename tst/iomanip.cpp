#include <stdexcept>
#include <iostream>
#include <sstream>
#include "../src/Log.h"
#include "../src/IOManip.h"
#include "iomanip.h"

using namespace std;


#if (_TEST_ALL == 1) || (_TEST_GRP_IOMNP == 1) || (_TEST_IOMNP_GETCSTR == 1)
void test_iomnp_getcstr()
{
  stringstream sfd;

  sfd
    << "\"quote:\\\"\"" << ", "
    << "\"escape:\\\\\"" << ", "
    << "backslash:\\" << ", "
    << "\"space: \"" << ", "
    << "text" << ", "
    << "" << ",\n";
  sfd.clear();
  sfd.seekg(0);

  cstrtype ct;
  ct.set_dfl();
  ct.set_brk(",");
  string s, d;
  sfd >> get_cstr(s, ct) >> setw(1) >> d;
  if ((s != "quote:\"") || (d != ","))
    logexc << "get_cstr() failed to scan field string \"quote:\\\"\"" << endl;
  sfd >> get_cstr(s, ct) >> setw(1) >> d;
  if ((s != "escape:\\") || (d != ","))
    logexc << "get_cstr() failed to scan field string \"escape:\\\\\"" << endl;
  sfd >> get_cstr(s, ct) >> setw(1) >> d;
  if ((s != "backslash:\\") || (d != ","))
    logexc << "get_cstr() failed to scan field string \"backslash:\\\"" << endl;
  sfd >> get_cstr(s, ct) >> setw(1) >> d;
  if ((s != "space: ") || (d != ","))
    logexc << "get_cstr() failed to scan field string \"space: \"" << endl;
  sfd >> get_cstr(s, ct) >> setw(1) >> d;
  if ((s != "text") || (d != ","))
    logexc << "get_cstr() failed to scan plain text field string" << endl;
  sfd >> get_cstr(s, ct) >> setw(1) >> d;
  if ((s != "") || (d != ","))
    logexc << "get_cstr() failed to scan empty field string" << endl;
}
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_IOMNP == 1) || (_TEST_IOMNP_PUTCSTR == 1)
void test_iomnp_putcstr()
{
  stringstream sfd;
  cstrtype ct;
  ct.set_dfl();
  ct.set_brk(",");
  sfd
    << put_cstr(string("quote:\""), ct) << ", "
    << put_cstr(string("escape:\\"), ct) << ", "
    << put_cstr(string("space: "), ct) << ", "
    << put_cstr(string("text"), ct) << ", "
    << put_cstr(string(""), ct) << ",\n";
  string s = sfd.str();
  if (s != "\"quote:\\\"\", escape:\\, \"space: \", text, ,\n")
    logexc << "put_str() format error detected" << endl;
}
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_IOMNP == 1) || (_TEST_IOMNP_GETCINT == 1)
void test_iomnp_getcint()
{
  stringstream sfd("123 -999 0777 0x123 09 078 0");
  int i;
  if (!(sfd >> get_cint(i)) || (i != 123))
    logexc << "get_cint failed to scan 123" << endl;
  if (!(sfd >> get_cint(i)) || (i != -999))
    logexc << "get_cint failed to scan -999" << endl;
  if (!(sfd >> get_cint(i)) || (i != 0777))
    logexc << "get_cint failed to scan 0777" << endl;
  if (!(sfd >> get_cint(i)) || (i != 0x123))
    logexc << "get_cint failed to scan 0x123" << endl;
  if (!(sfd >> get_cint(i)) || (i != 0))
    logexc << "get_cint failed to scan 0" << endl;
  if (!(sfd >> get_cint(i)) || (i != 9))
    logexc << "get_cint failed to scan 9" << endl;
  if (!(sfd >> get_cint(i)) || (i != 07))
    logexc << "get_cint failed to scan 07" << endl;
  if (!(sfd >> get_cint(i)) || (i != 8))
    logexc << "get_cint failed to scan 8" << endl;
  if (!(sfd >> get_cint(i)) || (i != 0))
    logexc << "get_cint failed to scan final 0" << endl;
}
#endif
