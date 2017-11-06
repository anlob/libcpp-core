#ifndef SSL_H_INCLUDED
#define SSL_H_INCLUDED

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>


class SSLInit
{
public:
  SSLInit();
private:
  static bool init_;
};

class SSLH: protected SSLInit
{
public:
  const unsigned _FD_NOCLOSE = 1;

  SSLH(int sockfd, unsigned opt = 0);
  SSLH(int pfd[2], unsigned opt = 0);
  ~SSLH();
private:
  SSL *CreateH();
  unsigned opt_;
  SSL *ssl_;
  SSL_CTX *ctx_;
  BIO *bio_;
  BIO *bio2_;
};


#endif // SSL_H_INCLUDED
