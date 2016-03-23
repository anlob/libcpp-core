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
