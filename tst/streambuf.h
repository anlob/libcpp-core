#ifndef TEST_STREAMBUF_H_INCLUDED
#define TEST_STREAMBUF_H_INCLUDED

#include "conf.h"


#if (_TEST_ALL == 1) || (_TEST_GRP_STMBF == 1) || (_TEST_STMBF_INP == 1)
void test_stmbf_inp();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_STMBF == 1) || (_TEST_STMBF_OUT == 1)
void test_stmbf_out();
#endif


#endif // TEST_STREAMBUF_H_INCLUDED

