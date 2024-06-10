#ifndef JET_LOGGER_H
#define JET_LOGGER_H

#include <cstdio>
//#include <format>
#include <iostream>

namespace jet::Logger
{
template <typename... Args> inline void Error(const std::format_string<Args...> fmt, Args &&...args);
}

inline std::string className(const std::string &prettyFunction, const std::string &func)
{
  size_t begin = prettyFunction.find("jet::") + 5;
  if (begin == std::string::npos)
  {
    jet::Logger::Error("Failed to parse class name");
    return "";
  }

  size_t end = prettyFunction.find(func);
  end = end - begin - 2;

  return prettyFunction.substr(begin, end);
}

#define __CLASS_NAME__ className(__FUNCTION__, __FUNCDNAME__)

namespace jet
{

namespace Logger
{

namespace Color
{

enum Code
{
  RESET = 01,

  FG_BLACK = 30,
  FG_RED = 31,
  FG_GREEN = 32,
  FG_YELLOW = 33,
  FG_BLUE = 34,
  FG_MAGENTA = 35,
  FG_CYAN = 36,
  FG_WHITE = 37,
  FG_DEFAULT = 39,

  BG_BLACK = 40,
  BG_RED = 41,
  BG_GREEN = 42,
  BG_YELLOW = 43,
  BG_BLUE = 44,
  BG_MAGENTA = 45,
  BG_CYAN = 46,
  BG_WHITE = 47,
  BG_DEFAULT = 49,
};

class Mod
{
public:
  static Mod Red()
  {
    return Mod(Color::FG_RED);
  }
  static Mod Green()
  {
    return Mod(Color::FG_GREEN);
  }
  static Mod Yellow()
  {
    return Mod(Color::FG_YELLOW);
  }
  static Mod Blue()
  {
    return Mod(Color::FG_BLUE);
  }
  static Mod Magenta()
  {
    return Mod(Color::FG_MAGENTA);
  }
  static Mod Cyan()
  {
    return Mod(Color::FG_CYAN);
  }
  static Mod White()
  {
    return Mod(Color::FG_WHITE);
  }
  static Mod Default()
  {
    return Mod(Color::FG_DEFAULT);
  }

  Code code;

public:
  Mod(Code pCode) : code(pCode)
  {
  }
  friend std::ostream &operator<<(std::ostream &os, const Mod &mod)
  {
    return os << "\033[" << mod.code << "m";
  }
};

} // namespace Color

template <typename... Args> inline void Message(const std::format_string<Args...> fmt, Args &&...args)
{
  std::cout << "\t- " << std::vformat(fmt.get(), std::make_format_args(args...)) << std::endl;
}

template <typename... Args> inline void Info(const std::format_string<Args...> fmt, Args &&...args)
{
  std::cout << Color::Mod::Green() << "[INFO]: " << Color::Mod::Default()
            << std::vformat(fmt.get(), std::make_format_args(args...)) << std::endl;
}

template <typename... Args> inline void Warning(const std::format_string<Args...> fmt, Args &&...args)
{
  std::cout << Color::Mod::Yellow() << "[WARNING]: " << Color::Mod::Default()
            << std::vformat(fmt.get(), std::make_format_args(args...)) << std::endl;
}

template <typename... Args> inline void Error(const std::format_string<Args...> fmt, Args &&...args)
{
  std::cout << Color::Mod::Red() << "[ERROR]: " << Color::Mod::Default()
            << std::vformat(fmt.get(), std::make_format_args(args...)) << std::endl;
}

#define CHECK_IN(...) Logger::Display(__FUNCTION__)

inline void Display(const std::string &m)
{
  std::cout << Color::Mod::Cyan() << "[DISPLAY]: " << Color::Mod::Default() << m << std::endl;
}

} // namespace Logger

} // namespace jet

#endif // JET_LOGGER_H
