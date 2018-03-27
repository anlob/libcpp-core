#include <string>
#include <cstring>
#include <stdexcept>
#include <istream>
#include <ostream>
#include <iostream>
#include <jsoncpp/json/json.h>
#include "../src/Log.h"
#include "json.h"

using namespace std;


#if (_TEST_ALL == 1) || (_TEST_GRP_JSON == 1) || (_TEST_JSON_RAW == 1)
void test_json_raw()
{
static const char *data_bad =
"\
foobar\
";
static const char *data =
"\
// Configuration options\n\
{\n\
    // Default encoding for text\n\
    \"encoding\" : \"UTF-8\",\n\
    \n\
    // Plug-ins loaded at start-up\n\
    \"plug-ins\" : [\n\
        \"python\",\n\
        \"c++\",  // trailing comment\n\
        \"ruby\"\n\
        ],\n\
        \n\
    // Tab indent size\n\
    // (multi-line comment)\n\
    \"indent\" : { /*embedded comment*/ \"length\" : 3, \"use_space\": true }\n\
}\
";
  {
    istringstream is(data_bad);
    Json::Value root;
    try {
      is >> root;
    } catch (...) {
      is.setstate(ios::failbit);
    }
    if (is)
      logexc << "no parse error reported when reading bad json data" << endl;
  }

  istringstream is(data);
  Json::Value root;
  is >> root;
}
#endif
