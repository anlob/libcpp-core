#ifndef TEST_IOMANIP_H_INCLUDED
#define TEST_IOMANIP_H_INCLUDED

#include "conf.h"


#if (_TEST_ALL == 1) || (_TEST_GRP_IOMNP == 1) || (_TEST_IOMNP_GETCSTR == 1)
void test_iomnp_getcstr();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_IOMNP == 1) || (_TEST_IOMNP_PUTCSTR == 1)
void test_iomnp_putcstr();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_IOMNP == 1) || (_TEST_IOMNP_GETCINT == 1)
void test_iomnp_getcint();
#endif


#endif // TEST_IOMANIP_H_INCLUDED
