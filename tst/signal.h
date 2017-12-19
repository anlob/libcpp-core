#ifndef TEST_SIGNAL_H_INCLUDED
#define TEST_SIGNAL_H_INCLUDED

#include "conf.h"


#if (_TEST_ALL == 1) || (_TEST_GRP_SIG == 1) || (_TEST_SIG_MASK == 1)
void test_sig_mask();
#endif


#endif // TEST_SIGNAL_H_INCLUDED

