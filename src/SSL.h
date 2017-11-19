#ifndef SSL_H_INCLUDED
#define SSL_H_INCLUDED

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <istream>
#include <ostream>
#include <memory>
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

  bool rdshut();
  bool wrshut();

  bool connect();
  bool connected();
  bool verify();
  std::streamsize read(void *buf, std::size_t bufsz);
  std::streamsize write(const void *buf, std::size_t bufsz);
  bool shutrd();
  bool shutwr();
  bool shutdown() { return shutwr() && shutrd(); }

private:
  SSLH &move(SSLH &from);
  SSL *CreateH();

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
class BasicSSLSock:
  protected SSLH,
  private std::unique_ptr<BasicIStreamBuf<_E, _Tr> >,
  private std::unique_ptr<BasicOStreamBuf<_E, _Tr> >,
  public BasicSock<_E, _Tr>
{
  class _ISBf: public BasicIStreamBuf<_E, _Tr>
  {
  public:
    _ISBf(BasicSSLSock &sslsock): sslsock_(sslsock) { sslsock.std::unique_ptr<BasicIStreamBuf<_E, _Tr> >::reset(this); }
    virtual ~_ISBf() {}
    std::streamsize _fill(std::size_t bufsz) { return sslsock_.SSLH::read((void *) BasicIStreamBuf<_E, _Tr>::gptr(), bufsz * sizeof(_E)); }
    BasicSSLSock &sslsock_;
  };

  class _OSBf: public BasicOStreamBuf<_E, _Tr>
  {
  public:
    _OSBf(BasicSSLSock &sslsock): sslsock_(sslsock) { sslsock.std::unique_ptr<BasicOStreamBuf<_E, _Tr> >::reset(this); }
    virtual ~_OSBf() {}
    std::streamsize _flush() { return sslsock_.SSLH::write(BasicOStreamBuf<_E, _Tr>::pbase(), (BasicOStreamBuf<_E, _Tr>::pptr() - BasicOStreamBuf<_E, _Tr>::pbase()) * sizeof(_E)); }
    BasicSSLSock &sslsock_;
  };

public:
  BasicSSLSock(int sockfd):
    SSLH(sockfd),
    BasicSock<_E, _Tr>(new _ISBf(*this), new _OSBf(*this))
  {
    std::unique_ptr<BasicIStreamBuf<_E, _Tr> >::release();
    std::unique_ptr<BasicOStreamBuf<_E, _Tr> >::release();
    connect();
  }
  BasicSSLSock(FD &&sfd): BasicSSLSock((int) sfd) { sfd.Detach(); }
  BasicSSLSock(struct sockaddr *addr): BasicSSLSock(SockFN::Connect(addr)) {}
  BasicSSLSock(const char *addr): BasicSSLSock(SockFN::Connect(addr)) {}
  virtual ~BasicSSLSock() {}

  std::basic_istream<_E, _Tr> &in() { return BasicSock<_E, _Tr>::in(); }
  std::basic_ostream<_E, _Tr> &out() { return BasicSock<_E, _Tr>::out(); }
  operator std::basic_istream<_E, _Tr> &() { return in(); }
  operator std::basic_ostream<_E, _Tr> &() { return out(); }

  template<typename _T> std::basic_istream<_E, _Tr> &operator>>(_T _v) { return in() >> _v; }
  template<typename _T> std::basic_ostream<_E, _Tr> &operator<<(_T _v) { return out() << _v; }

  std::streamsize readsome(_E *_s, std::streamsize _n) { return in().readsome(_s, _n); }
  std::basic_istream<_E, _Tr> &read(_E *_s, std::streamsize _n) { return in().read(_s, _n); }
  std::basic_ostream<_E, _Tr> &write(const _E *_s, std::streamsize _n) { return out().write(_s, _n); }

  typename std::basic_istream<_E, _Tr>::int_type get() { return in().get(); }
  std::basic_istream<_E, _Tr> &get(_E &_c) { return in().get(_c); }
  std::basic_istream<_E, _Tr> &putback(_E _c) { return in().putback(_c); }
  std::basic_ostream<_E, _Tr> &put(_E _c) { return out().put(_c); }
  std::basic_ostream<_E, _Tr> &flush() { return out().flush(); }
};

typedef BasicSSLSock<char> SSLSock;
typedef BasicSSLSock<wchar_t> SSLWSock;


#endif // SSL_H_INCLUDED
