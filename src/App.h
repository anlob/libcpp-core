/** \file App.h
 * Application stuff.
 *
 * \author Andreas Lobbes, andreas.lobbes@gmail.com
 * \date 2016-2018
 * \copyright GPLV3 and above + GCC RTL Exception 3.1
 * \see http://www.gnu.org/licenses
 */
#ifndef APP_H_INCLUDED
#define APP_H_INCLUDED

#include <string>


class App
{
public:
  static const std::string &get_name();
  static const std::string &get_homedir();
  static const std::string &get_confdir();
};


#endif // APP_H_INCLUDED
