#ifndef TEST_SOCK_H_INCLUDED
#define TEST_SOCK_H_INCLUDED

#include "conf.h"


#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_IPV6CONN == 1)
void test_sock_ipv6conn();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_HTTPREQ == 1)
void test_sock_httpreq();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_SHUT == 1)
void test_sock_shut();
#endif


#endif // TEST_SOCK_H_INCLUDED
