#ifndef TEST_SSL_H_INCLUDED
#define TEST_SSL_H_INCLUDED

#include "conf.h"


#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_CONN == 1)
void test_ssl_conn();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_SHUT == 1)
void test_ssl_shut();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_VIFY == 1)
void test_ssl_vify();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_RECONN == 1)
void test_ssl_reconn();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_HTTPREQ == 1)
void test_ssl_httpreq();
#endif
#if (_TEST_ALL == 1) || (_TEST_GRP_SSL == 1) || (_TEST_SSL_SOCK == 1)
void test_ssl_sock();
#endif


#endif // TEST_SSL_H_INCLUDED
