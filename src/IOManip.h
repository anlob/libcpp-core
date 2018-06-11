/** \file IOManip.h
 * IO manipulator stuff.
 *
 * \author Andreas Lobbes, andreas.lobbes@gmail.com
 * \date 2016-2018
 * \copyright GPLV3 and above + GCC RTL Exception 3.1
 * \see http://www.gnu.org/licenses
 */
#ifndef IOMANIP_H_INCLUDED
#define IOMANIP_H_INCLUDED

#include <istream>
#include <ostream>
#include <iostream>
#include <iomanip>
#include <iosfwd>
#include <string>
#include <locale>


template<typename _E, typename _Tr = std::char_traits<_E> > struct BasicCStrType;
template<typename _E, typename _Tr = std::char_traits<_E> > class BasicGetCStr;
template<typename _E, typename _Tr = std::char_traits<_E> > class BasicPutCStr;
template<typename _E, typename _Tr = std::char_traits<_E> > std::basic_istream<_E, _Tr> &operator>>(std::basic_istream<_E, _Tr> &is, const BasicGetCStr<_E, _Tr> &im);
template<typename _E, typename _Tr = std::char_traits<_E> > std::basic_ostream<_E, _Tr> &operator<<(std::basic_ostream<_E, _Tr> &os, const BasicPutCStr<_E, _Tr> &om);


template<typename _E, typename _Tr>
struct BasicCStrType
{
  _E quot;
  _E esc;
  const _E *brk;
  unsigned nbrk;
  bool isbrk(const _E &c) const { return _Tr::find(brk, nbrk, c) != nullptr; }

  void set_dfl() { *this = *dflt; }
  void set_brk(const _E *s) { brk = s; nbrk = (s != nullptr) ? (unsigned) _Tr::length(s) : 0U; }

  static const BasicCStrType *const dflt;
};

typedef BasicCStrType<char> cstrtype;
typedef BasicCStrType<wchar_t> wcstrtype;


template<typename _E, typename _Tr>
class BasicGetCStr
{
  friend std::basic_istream<_E, _Tr> &operator>> <_E, _Tr>(std::basic_istream<_E, _Tr> &in, const BasicGetCStr<_E, _Tr> &im);

public:
  BasicGetCStr(std::basic_string<_E, _Tr> &s, const BasicCStrType<_E, _Tr> &ct = *BasicCStrType<_E, _Tr>::dflt): s_(s), ct_(ct) {}

protected:
  std::basic_string<_E, _Tr> &s_;
  const BasicCStrType<_E, _Tr> &ct_;
};

typedef BasicGetCStr<char> get_cstr;
typedef BasicGetCStr<wchar_t> get_wcstr;


template<typename _E, typename _Tr>
class BasicPutCStr
{
  friend std::basic_ostream<_E, _Tr> &operator<< <_E, _Tr>(std::basic_ostream<_E, _Tr> &out, const BasicPutCStr<_E, _Tr> &om);

public:
  BasicPutCStr(const std::basic_string<_E, _Tr> &s, const BasicCStrType<_E, _Tr> &ct = *BasicCStrType<_E, _Tr>::dflt): s_(s), ct_(ct) {}

protected:
  const std::basic_string<_E, _Tr> &s_;
  const BasicCStrType<_E, _Tr> &ct_;
};

typedef BasicPutCStr<char> put_cstr;
typedef BasicPutCStr<wchar_t> put_wcstr;


template<typename _E, typename _Tr = std::char_traits<_E> >
std::basic_istream<_E, _Tr> &operator>>(std::basic_istream<_E, _Tr> &is, const BasicGetCStr<_E, _Tr> &im)
{
  const std::ctype<_E> &ct = std::use_facet<std::ctype<_E> >(is.getloc());

  _E c;
  do if (!is.get(c)) return is;
  while ((is.flags() & std::ios::skipws) && ct.is(std::ctype<_E>::space, c));

  im.s_.clear();
  if (c != im.ct_.quot) while (1) {
      if (im.ct_.isbrk(c))
        return is.putback(c);
      im.s_.push_back(c);
      if (!is.get(c)) {
        is.clear(is.rdstate() & ~std::ios::failbit);
        return is;
      }
      if ((is.flags() & std::ios::skipws) && ct.is(std::ctype<_E>::space, c))
        return is.putback(c);
  }

  bool esc = false;
  while (1) {
    if (!is.get(c)) {
      if (esc && (im.ct_.esc == im.ct_.quot))
        is.clear(is.rdstate() & ~std::ios::failbit);
      return is;
    }
    if (esc) {
      esc = false;
      if (c != im.ct_.quot) {
        if (im.ct_.esc == im.ct_.quot)
          return is.putback(c);
        im.s_.push_back(im.ct_.esc);
        if (c == im.ct_.esc)
          continue;
      }
      im.s_.push_back(c);
      continue;
    }
    if ((esc = (c == im.ct_.esc)))
      continue;
    if (c == im.ct_.quot)
      return is;
    im.s_.push_back(c);
  }

  return is;
}

template<typename _E, typename _Tr = std::char_traits<_E> >
std::basic_ostream<_E, _Tr> &operator<<(std::basic_ostream<_E, _Tr> &os, const BasicPutCStr<_E, _Tr> &om)
{
  const std::ctype<_E> &ct = std::use_facet<std::ctype<_E> >(os.getloc());

  bool fold = false;
  if (om.ct_.nbrk == 0)
    fold = true;
  else for (typename std::basic_string<_E, _Tr>::const_iterator it = om.s_.begin(); !fold && (it != om.s_.end()); ++it)
    fold = ((*it == om.ct_.quot) || ct.is(std::ctype<_E>::space, *it) || om.ct_.isbrk(*it));

  if (!fold)
    return os << om.s_;

  if (!os.put(om.ct_.quot))
    return os;
  for (typename std::basic_string<_E, _Tr>::const_iterator it = om.s_.begin(); it != om.s_.end(); ++it) {
    if ((*it == om.ct_.quot) && !os.put(om.ct_.esc))
      return os;
    if (!os.put(*it))
      return os;
  }
  if (!os.put(om.ct_.quot))
    return os;

  return os;
}


template<typename _V> struct BasicCIntType
{
  BasicCIntType(_V &v): val(v) {}
  _V &val;
};

template<typename _V> BasicCIntType<_V> get_cint(_V &v) { return BasicCIntType<_V>(v); }

template<typename _V, typename _E, typename _Tr = std::char_traits<_E> > std::basic_istream<_E, _Tr> &operator>>(std::basic_istream<_E, _Tr> &is, const BasicCIntType<_V> &ci)
{
  const std::ctype<_E> &ct = std::use_facet<std::ctype<_E> >(is.getloc());

  _E c;
  do if (!is.get(c)) return is;
  while ((is.flags() & std::ios::skipws) && ct.is(std::ctype<_E>::space, c));

  if (c != ct.widen('0')) {
    is.putback(c);
    return is >> std::dec >> ci.val;
  }
  if (!is.get(c)) {
    ci.val = 0;
    is.clear(is.rdstate() & ~std::ios::failbit);
    return is;
  }
  if ((c == ct.widen('x')) || (c == ct.widen('X')))
    return is >> std::hex >> ci.val;
  is.putback(c);
  if (!ct.is(std::ctype<_E>::digit, c) || !(is >> std::oct >> ci.val)) {
    ci.val = 0;
    is.clear(is.rdstate() & ~std::ios::failbit);
  }
  return is;
}


#endif // IOMANIP_H_INCLUDED
