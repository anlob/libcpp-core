#include "Log.h"
#include "SSL.h"

using namespace std;


bool SSLInit::init_ = false;

SSLInit::SSLInit()
{
  if (init_)
    return;

  SSL_library_init();
  ERR_load_BIO_strings();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
}


SSLH::SSLH(int sockfd, unsigned opt /* = 0*/): opt_(opt), ssl_(nullptr), ctx_(nullptr), bio_(nullptr), bio2_(nullptr)
{
  bio_ = BIO_new(BIO_s_socket());
  if (bio_ == nullptr)
    logexc << "BIO_new(BIO_s_socket()) failed unexpectedly" << endl;

  CreateH();
  BIO_set_fd(bio_, sockfd, !(opt & _FD_NOCLOSE));
  SSL_set_bio(ssl_, bio_, bio_);
}
SSLH::SSLH(int pfd[2], unsigned opt /* = 0*/): opt_(opt), ssl_(nullptr), ctx_(nullptr), bio_(nullptr), bio2_(nullptr)
{
  bio_ = BIO_new(BIO_s_fd());
  bio2_ = BIO_new(BIO_s_fd());
  if ((bio_ == nullptr) || (bio2_ == nullptr))
    logexc << "BIO_new(BIO_s_fd()) failed unexpectedly" << endl;

  CreateH();
  BIO_set_fd(bio_, pfd[0], !(opt & _FD_NOCLOSE));
  BIO_set_fd(bio2_, pfd[1], !(opt & _FD_NOCLOSE));
  SSL_set_bio(ssl_, bio_, bio2_);
}

SSLH::~SSLH()
{
  if (ssl_ != nullptr)
    SSL_free(ssl_);
  if (ctx_ != nullptr)
    SSL_CTX_free(ctx_);
  if (bio_ != nullptr)
    BIO_free(bio_);
  if (bio2_ != nullptr)
    BIO_free(bio2_);
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
  ssl_ = SSL_new(ctx_);
  if (ssl_ == nullptr)
    logexc << "SSL_new(ctx) failed unexpectedly" << endl;
  return ssl_;
}
