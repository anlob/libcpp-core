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
  virtual ~SSLH();

  /**
   * last SSL_ERROR_XXX reported by ssl layer
   *
   * \return int
   */
  int lasterr() const { return lasterr_; }

  bool connect();
  bool shutdown();

private:
  SSL *CreateH();

  bool conn_;     /**< connected flag */
  bool vify_;     /**< connection verified flag */
  int lasterr_;   /**< last SSL_ERROR_XXX reported by ssl layer */
  unsigned opt_;
  SSL *ssl_;
  SSL_CTX *ctx_;
  BIO *bio_;
  BIO *bio2_;
};


#endif // SSL_H_INCLUDED
