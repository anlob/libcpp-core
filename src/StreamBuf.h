#ifndef STREAMBUF_H_INCLUDED
#define STREAMBUF_H_INCLUDED

#include <iostream>
#include <vector>
#include <climits>


template<typename _E, typename _Tr = std::char_traits<_E> >
class BasicOStreamBuf: public std::basic_streambuf<_E, _Tr>
{
public:
  BasicOStreamBuf(std::size_t bufsz = PIPE_BUF)
  {
    buf_.resize(bufsz);
    std::basic_streambuf<_E, _Tr>::setp(&buf_.front(), (&buf_.back()) + 1);
  }
  virtual ~BasicOStreamBuf() {}
protected:
  virtual typename std::char_traits<_E>::int_type overflow(typename std::char_traits<_E>::int_type c = std::char_traits<_E>::eof())
  {
    if (c != std::char_traits<_E>::eof()) {
      *std::basic_streambuf<_E, _Tr>::pptr() = std::char_traits<_E>::to_char_type(c);
      std::basic_streambuf<_E, _Tr>::pbump(1);
    }
    if ((std::basic_streambuf<_E, _Tr>::pptr() == std::basic_streambuf<_E, _Tr>::epptr()) && (sync() < 0))
      return std::char_traits<_E>::eof();
    return std::char_traits<_E>::not_eof(c);
  }
  virtual int sync()
  {
    if (_flush() < 0)
      return -1;
    std::basic_streambuf<_E, _Tr>::setp(&buf_.front(), (&buf_.back()) + 1);
    return 0;
  }
private:
  /**
   * \brief flush buffer from pbase() up to pptr()
   * \return >= 0 on success, -1 on failure
   */
  virtual std::streamsize _flush() { return -1; }
  std::vector<_E> buf_;
};

template<typename _E, typename _Tr = std::char_traits<_E> >
class BasicIStreamBuf: public std::basic_streambuf<_E, _Tr>
{
public:
  BasicIStreamBuf(std::size_t bufsz = PIPE_BUF, std::size_t pbacksz = 1)
  {
    buf_.resize((bfsz_ = bufsz) + (pbsz_ = pbacksz));
    std::basic_streambuf<_E, _Tr>::setg(&buf_.front(), &buf_.front() + pbacksz, &buf_.front() + pbacksz);
  }
  virtual ~BasicIStreamBuf() {}
protected:
  virtual typename std::char_traits<_E>::int_type underflow()
  {
    std::basic_streambuf<_E, _Tr>::setg(&buf_.front(), &buf_.front() + pbsz_, &buf_.front() + pbsz_);
    int n = _fill(bfsz_);
    if (n <= 0)
      return std::char_traits<_E>::eof();
    std::basic_streambuf<_E, _Tr>::setg(&buf_.front(), &buf_.front() + pbsz_, &buf_.front() + pbsz_ + (unsigned) n);
    return std::char_traits<_E>::to_int_type(*std::basic_streambuf<_E, _Tr>::gptr());
  }
private:
  /**
   * \brief fill buffer at gptr() up to \a bufsz bytes
   * \param bufsz max number of elements that can be read
   * \return number of elements read, -1 on failure, 0 indicates EOF
   */
  virtual std::streamsize _fill(std::size_t bufsz) { return 0; }
  std::vector<_E> buf_;
  std::size_t bfsz_;
  std::size_t pbsz_;
};

typedef BasicOStreamBuf<char> OStreamBuf;
typedef BasicOStreamBuf<wchar_t> WOStreamBuf;
typedef BasicIStreamBuf<char> IStreamBuf;
typedef BasicIStreamBuf<wchar_t> WIStreamBuf;


#endif // STREAMBUF_H_INCLUDED
