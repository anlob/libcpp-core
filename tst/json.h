#ifndef TEST_JSON_H_INCLUDED
#define TEST_JSON_H_INCLUDED

#include "conf.h"


#if (_TEST_ALL == 1) || (_TEST_GRP_JSON == 1) || (_TEST_JSON_RAW == 1)
void test_json_raw();
#endif


#endif // TEST_JSON_H_INCLUDED
