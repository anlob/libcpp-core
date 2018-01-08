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
#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_LISTEN == 1)
void test_sock_listen();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_ADDRSTR == 1)
void test_sock_addrstr();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_ADDRLST == 1)
void test_sock_addrlst();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_NETADDR == 1)
void test_sock_netaddr();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SOCK == 1) || (_TEST_SOCK_NETMASK == 1)
void test_sock_netmask();
#endif


#endif // TEST_SOCK_H_INCLUDED
