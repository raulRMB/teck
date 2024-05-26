#ifndef JET_LOGGER_H
#define JET_LOGGER_H

#include <format>
#include <iostream>
 
namespace jet
{

namespace Logger
{
 
template<typename... Args>
inline void Info(const std::format_string<Args...> fmt, Args&&... args)
{
    std::cout << "\033[1;31m[INFO]:" << std::vformat(fmt.get(), std::make_format_args(args...)) << "\033[0m\n";
}
 
template<typename... Args>
inline void Warning(const std::format_string<Args...> fmt, Args&&... args)
{
    std::cout << "[INFO]:" << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n';
}
 
template<typename... Args>
inline void Error(const std::format_string<Args...> fmt, Args&&... args)
{
    std::cout << "[INFO]:" << std::vformat(fmt.get(), std::make_format_args(args...)) << '\n';
}

 
}

}

#endif //JET_LOGGER_H
