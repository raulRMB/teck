#include "logger.h"
#include <format>
#include <cstdarg>
#include <cassert>

namespace jet
{

namespace Logger
{

void Info(std::string fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    std::string buffer = std::vformat(fmt, std::make_format_args(args));
    va_end(args);

    std::string outString = "[INFO]: ";
    std::cout << outString << std::endl;
}

void Warning(std::string&& fmt, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsprintf(buffer, format, args);
    va_end(args);
    std::string outString = "[WARNING]: ";
    outString += buffer;
    std::cout << outString << std::endl;
}

void Error(std::string&& fmt, ...)
{
    va_list args;
    va_start(args, format);
    char* buffer = new char[16384];
    vsprintf(buffer, format, args);
    va_end(args);
    std::string outString = "[ERROR]: ";
    outString += buffer;
    std::cout << outString << std::endl;
    assert(false);
    delete[] buffer;
} 
 
}
