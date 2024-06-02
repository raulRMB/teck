#ifndef JET_LOGGER_H
#define JET_LOGGER_H

#include <cstdio>
#include <format>
#include <iostream>
#include <sstream>

#include "dynamic-array.h"

namespace jet {

namespace Logger {

namespace Color {

enum Code {
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

class Mod {
 public:
  static Mod Red() { return Mod(Color::FG_RED); }
  static Mod Green() { return Mod(Color::FG_GREEN); }
  static Mod Yellow() { return Mod(Color::FG_YELLOW); }
  static Mod Blue() { return Mod(Color::FG_BLUE); }
  static Mod Magenta() { return Mod(Color::FG_MAGENTA); }
  static Mod Cyan() { return Mod(Color::FG_CYAN); }
  static Mod White() { return Mod(Color::FG_WHITE); }
  static Mod Default() { return Mod(Color::FG_DEFAULT); }

  Code code;

 public:
  Mod(Code pCode) : code(pCode) {}
  friend std::ostream& operator<<(std::ostream& os, const Mod& mod) {
    return os << "\033[" << mod.code << "m";
  }
};

}  // namespace Color

template <typename... Args>
inline void Message(const std::format_string<Args...> fmt, Args&&... args) {
  std::cout << Color::Mod::Cyan() << "\t- "
            << std::vformat(fmt.get(), std::make_format_args(args...))
            << Color::Mod::Default() << std::endl;
}

template <typename... Args>
inline void Info(const std::format_string<Args...> fmt, Args&&... args) {
  std::cout << Color::Mod::Green() << "[INFO]: " << Color::Mod::Default()
            << std::vformat(fmt.get(), std::make_format_args(args...))
            << std::endl;
}

template <typename... Args>
inline void Warning(const std::format_string<Args...> fmt, Args&&... args) {
  std::cout << Color::Mod::Yellow() << "[WARNING]: " << Color::Mod::Default()
            << std::vformat(fmt.get(), std::make_format_args(args...))
            << std::endl;
}

template <typename... Args>
inline void Error(const std::format_string<Args...> fmt, Args&&... args) {
  std::cout << Color::Mod::Red() << "[ERROR]: " << Color::Mod::Default()
            << std::vformat(fmt.get(), std::make_format_args(args...))
            << std::endl;
}

#define CHECK_IN(...) Logger::Display(__PRETTY_FUNCTION__)

inline void Display(const std::string& info) {
  DynamicArray<std::string> tokens = DynamicArray<std::string>();
  std::stringstream ss(info);
  std::string token;

  while (std::getline(ss, token, ':')) {
    tokens.PushBack(token);
  }

  std::string functionName =
      tokens[tokens.Size() - 3] + ":" + tokens[tokens.Size() - 1];

  std::cout << Color::Mod::Yellow() << "[DISPLAY]: " << Color::Mod::Default()
            << functionName << std::endl;
}

}  // namespace Logger

}  // namespace jet

#endif  // JET_LOGGER_H
