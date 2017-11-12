#ifndef TEST_SSL_H_INCLUDED
#define TEST_SSL_H_INCLUDED

#include "conf.h"


#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_CONN == 1)
void test_ssl_conn();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_SHUT == 1)
void test_ssl_shut();
#endif


#endif // TEST_SSL_H_INCLUDED
