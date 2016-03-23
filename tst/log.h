#ifndef TEST_LOG_H_INCLUDED
#define TEST_LOG_H_INCLUDED

#include "conf.h"


#if (_TEST_ALL == 1) || (_TEST_GRP_LOG == 1) || (_TEST_LOG_ERR == 1)
void test_log_err();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_LOG == 1) || (_TEST_LOG_EXC == 1)
void test_log_exc();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_LOG == 1) || (_TEST_LOG_SXC == 1)
void test_log_sxc();
#endif


#endif // TEST_LOG_H_INCLUDED
