#ifndef CONF_H_INCLUDED
#define CONF_H_INCLUDED


// enable all tests
#define _TEST_ALL 1

// group test enable macros
#define _TEST_GRP_LOG   0
#define _TEST_GRP_APP   0
#define _TEST_GRP_SOCK  0
#define _TEST_GRP_PIPE  0
#define _TEST_GRP_PROC  0
#define _TEST_GRP_SIG   0
#define _TEST_GRP_IOMNP 0
#define _TEST_GRP_STMBF 0
#define _TEST_GRP_SSL   0

// enable particular log test
#define _TEST_LOG_ERR   0
#define _TEST_LOG_EXC   0
#define _TEST_LOG_SXC   0

// enable particular app test
#define _TEST_APP_HOMEDIR   0
#define _TEST_APP_CONFDIR   0

// enable particular socket test
#define _TEST_SOCK_IPV6CONN 0
#define _TEST_SOCK_HTTPREQ  0
#define _TEST_SOCK_SHUT     0

// enable particular pipe test
#define _TEST_PIPE_IO       0

// enable particular process test
#define _TEST_PROC_EXEC     0
#define _TEST_PROC_FN       0

// enable particular signal test
#define _TEST_SIG_MASK      0

// enable particular iomanip test
#define _TEST_IOMNP_GETCSTR 0
#define _TEST_IOMNP_PUTCSTR 0
#define _TEST_IOMNP_GETCINT 0

// enable particular streambuf test
#define _TEST_STMBF_INP     0
#define _TEST_STMBF_OUT     0

// enable particular ssl test
#define _TEST_SSL_CONN      0
#define _TEST_SSL_SHUT      0
#define _TEST_SSL_VIFY      0
#define _TEST_SSL_RECONN    0
#define _TEST_SSL_HTTPREQ   0
#define _TEST_SSL_SOCK      0


#endif // CONF_H_INCLUDED
