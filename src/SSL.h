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
  SSLH(const SSLH &from) = delete;
  SSLH(SSLH &&from) { move(from); }
  virtual ~SSLH();

  SSLH &operator=(const SSLH &from) = delete;
  SSLH &operator=(SSLH &&from) { return move(from); }

  int lasterr() const { return lasterr_; }
  int lasterr(int sslerr);

  bool connect();
  bool shutdown();
  bool verify();

private:
  SSLH &move(SSLH &from);
  SSL *CreateH();

  bool conn_;     /**< connected flag */
  bool vify_;     /**< connection verify flag */
  long vifyrslt_; /**< keeps return value of SSL_get_verify_result() */
  int lasterr_;   /**< last SSL_ERROR_XXX reported by ssl layer */
  std::string certname_;    /**< cert subject name */
  std::string certissuer_;  /**< cert issuer */
  unsigned opt_;
  SSL *ssl_;
  SSL_CTX *ctx_;
  BIO *bio_;
  BIO *bio2_;
};


#endif // SSL_H_INCLUDED
