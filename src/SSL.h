#ifndef SSL_H_INCLUDED
#define SSL_H_INCLUDED

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <istream>
#include <ostream>
#include "StreamBuf.h"
#include "Sock.h"


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
  const std::string &certname() const { return certname_; }
  const std::string &certissuer() const { return certissuer_; }

  bool connect();
  bool shutdown();
  bool verify();
  std::streamsize read(void *buf, std::size_t bufsz);
  std::streamsize write(const void *buf, std::size_t bufsz);

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


template<typename _E, typename _Tr = std::char_traits<_E> >
class BasicSSLSock: protected SSLH, private BasicIStreamBuf<_E, _Tr>, private BasicOStreamBuf<_E, _Tr>, private std::basic_istream<_E, _Tr>, private std::basic_ostream<_E, _Tr>, public BasicSock<_E, _Tr>
{
public:
  BasicSSLSock(int sockfd): SSLH(sockfd), std::basic_istream<_E, _Tr>((BasicIStreamBuf<_E, _Tr> *) this), std::basic_ostream<_E, _Tr>((BasicOStreamBuf<_E, _Tr> *) this), BasicSock<_E, _Tr>((std::basic_istream<_E, _Tr> *) this, (std::basic_ostream<_E, _Tr> *) this) { connect(); }
  BasicSSLSock(FD &&sfd): BasicSSLSock((int) sfd) { sfd.Detach(); }
  BasicSSLSock(struct sockaddr *addr): BasicSSLSock(SockFN::Connect(addr)) {}
  BasicSSLSock(const char *addr): BasicSSLSock(SockFN::Connect(addr)) {}
  virtual ~BasicSSLSock() {}
};

typedef BasicSSLSock<char> SSLSock;
typedef BasicSSLSock<wchar_t> SSLWSock;


#endif // SSL_H_INCLUDED
