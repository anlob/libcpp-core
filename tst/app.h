#ifndef TEST_APP_H_INCLUDED
#define TEST_APP_H_INCLUDED

#include "conf.h"


#if (_TEST_ALL == 1) || (_TEST_GRP_APP == 1) || (_TEST_APP_HOMEDIR == 1)
void test_app_homedir();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_APP == 1) || (_TEST_APP_CONFDIR == 1)
void test_app_confdir();
#endif


#endif // TEST_APP_H_INCLUDED
