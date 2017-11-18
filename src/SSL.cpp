#include "Log.h"
#include "SSL.h"

using namespace std;


bool SSLInit::init_ = false;

SSLInit::SSLInit()
{
  if (init_)
    return;

  init_ = true;
  SSL_library_init();
  ERR_load_BIO_strings();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
}


SSLH::SSLH(int sockfd, unsigned opt /* = 0*/): conn_(false), vify_(false), lasterr_(SSL_ERROR_NONE), opt_(opt), ssl_(nullptr), ctx_(nullptr), bio_(nullptr), bio2_(nullptr)
{
  bio_ = BIO_new(BIO_s_socket());
  if (bio_ == nullptr)
    logexc << "BIO_new(BIO_s_socket()) failed unexpectedly" << endl;

  CreateH();
  BIO_set_fd(bio_, sockfd, !(opt & _FD_NOCLOSE));
  SSL_set_bio(ssl_, bio_, bio_);
  BIO_up_ref(bio_);
}

SSLH::SSLH(int pfd[2], unsigned opt /* = 0*/): conn_(false), vify_(false), lasterr_(SSL_ERROR_NONE), opt_(opt), ssl_(nullptr), ctx_(nullptr), bio_(nullptr), bio2_(nullptr)
{
  bio_ = BIO_new(BIO_s_fd());
  bio2_ = BIO_new(BIO_s_fd());
  if ((bio_ == nullptr) || (bio2_ == nullptr))
    logexc << "BIO_new(BIO_s_fd()) failed unexpectedly" << endl;

  CreateH();
  BIO_set_fd(bio_, pfd[0], !(opt & _FD_NOCLOSE));
  BIO_set_fd(bio2_, pfd[1], !(opt & _FD_NOCLOSE));
  SSL_set_bio(ssl_, bio_, bio2_);
  BIO_up_ref(bio_);
  BIO_up_ref(bio2_);
}

SSLH::~SSLH()
{
  if (ssl_ != nullptr)
    SSL_free(ssl_);
  if (bio_ != nullptr)
    BIO_free(bio_);
  if (bio2_ != nullptr)
    BIO_free(bio2_);
  if (ctx_ != nullptr)
    SSL_CTX_free(ctx_);
}

SSLH &SSLH::move(SSLH &from)
{
  conn_ = from.conn_;
  vify_ = from.vify_;
  vifyrslt_ = from.vifyrslt_;
  lasterr_ = from.lasterr_;
  certname_ = from.certname_;
  certissuer_ = from.certissuer_;
  opt_ = from.opt_;
  ssl_ = from.ssl_;
  from.ssl_ = nullptr;
  ctx_ = from.ctx_;
  from.ctx_ = nullptr;
  bio_ = from.bio_;
  from.bio_ = nullptr;
  bio2_ = from.bio2_;
  from.bio2_ = nullptr;

  return *this;
}


int SSLH::lasterr(int sslerr)
{
  int e = lasterr_;
  if (sslerr >= 0)
    lasterr_ = sslerr;
  return e;
}


bool SSLH::connect()
{
  if (conn_)
    return true;

  while (1) {
    int n = SSL_connect(ssl_);
    if (n == 1)
      return conn_ = true;
    if (((n = SSL_get_error(ssl_, n)) == SSL_ERROR_WANT_READ) || (n == SSL_ERROR_WANT_WRITE))
      continue;

    logerr << "SSL_connect() failed with ssl error " << (lasterr_ = n) << endl;
    unsigned long e;
    while ((e = ERR_get_error()) != 0)
      logerr << "ssl errstk: " << ERR_error_string(e, nullptr) << endl;

    return false;
  }
}

bool SSLH::shutdown()
{
  if (!conn_)
    return true;

  bool wshut = false;
  while (1) {
    int n = SSL_shutdown(ssl_);
    if (n == 1)
      return !(conn_ = vify_ = false);
    if (n == 0) {
      wshut = true;
      continue;
    }
    if (((n = SSL_get_error(ssl_, n)) == SSL_ERROR_WANT_READ) || (n == SSL_ERROR_WANT_WRITE))
      continue;

    if (wshut) {
      logwrn << "graceful SSL_shutdown() failed with ssl error " << (lasterr_ = n) << endl;
      unsigned long e;
      while ((e = ERR_get_error()) != 0)
        logwrn << "ssl errstk: " << ERR_error_string(e, nullptr) << endl;
      return !(conn_ = vify_ = false);
    }

    logerr << "SSL_shutdown() failed with ssl error " << (lasterr_ = n) << endl;
    unsigned long e;
    while ((e = ERR_get_error()) != 0)
      logerr << "ssl errstk: " << ERR_error_string(e, nullptr) << endl;
    return false;
  }
}

bool SSLH::verify()
{
  if (!conn_)
    return false;

  if (!vify_) {
    certname_.clear();
    certissuer_.clear();
    X509 *cert = SSL_get_peer_certificate(ssl_);
    if (cert != nullptr) {
      char sbf[512];
      X509_NAME *cname = X509_get_subject_name(cert);
      if (cname != nullptr)
        certname_ = X509_NAME_oneline(cname, sbf, (int) sizeof(sbf));
      else
        logerr << "X509_get_subject_name() returned NULL" << endl;
      cname = X509_get_issuer_name(cert);
      if (cname != nullptr)
        certissuer_ = X509_NAME_oneline(cname, sbf, (int) sizeof(sbf));
      else
        logerr << "X509_get_issuer_name() returned NULL" << endl;
      if ((vifyrslt_ = SSL_get_verify_result(ssl_)) != X509_V_OK)
        logerr << "SSL_get_verify_result() returned " << vifyrslt_ << endl;
    }
    else
      logerr << "SSL_get_peer_certificate() returned NULL" << endl;
  }

  vify_ = true;
  return (vifyrslt_ == X509_V_OK) && !certname_.empty() && !certissuer_.empty();
}

streamsize SSLH::read(void *buf, size_t bufsz)
{
  while (1) {
    int n = SSL_read(ssl_, buf, bufsz);
    if (n > 0)
      return n;

    switch(n = SSL_get_error(ssl_, n)) {
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      continue;
    case SSL_ERROR_ZERO_RETURN:
      return 0; // EOF
    }

    logerr << "SSL_read() failed with ssl error " << (lasterr_ = n) << endl;
    unsigned long e;
    while ((e = ERR_get_error()) != 0)
      logerr << "ssl errstk: " << ERR_error_string(e, nullptr) << endl;
    return -1;
  }
}

streamsize SSLH::write(const void *buf, size_t bufsz)
{
  while (1) {
    int n = SSL_write(ssl_, buf, bufsz);
    if (n >= 0)
      return n;

    switch(n = SSL_get_error(ssl_, n)) {
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:
      continue;
    }

    logerr << "SSL_write() failed with ssl error " << (lasterr_ = n) << endl;
    unsigned long e;
    while ((e = ERR_get_error()) != 0)
      logerr << "ssl errstk: " << ERR_error_string(e, nullptr) << endl;
    return -1;
  }
}


SSL *SSLH::CreateH()
{
  const SSL_METHOD *meth = SSLv23_client_method();
  if (meth == nullptr)
    logexc << "SSLv23_client_method() failed unexpectedly" << endl;
  ctx_ = SSL_CTX_new(meth);
  if (ctx_ == nullptr)
    logexc << "SSL_CTX_new(method) failed unexpectedly" << endl;
  unsigned long ctxopt = SSL_CTX_get_options(ctx_);
  // use all bug workarounds of category harmless
  ctxopt |= SSL_OP_ALL;
#ifdef SSL_OP_NO_SSLv2
  // exclude SSLV2
  ctxopt |= SSL_OP_NO_SSLv2;
#endif
  SSL_CTX_set_options(ctx_, ctxopt);

  // TODO: be more flexible with various CA dirs.
  // "/etc/ssl/certs" should work with openssl installation.
  if (!SSL_CTX_load_verify_locations(ctx_, nullptr, "/etc/ssl/certs"))
    logexc << "SSL_CTX_load_verify_locations(/etc/ssl/certs) failed" << endl;

  ssl_ = SSL_new(ctx_);
  if (ssl_ == nullptr)
    logexc << "SSL_new(ctx) failed unexpectedly" << endl;
  return ssl_;
}
