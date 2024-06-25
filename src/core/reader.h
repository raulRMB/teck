#ifndef TECH_READER_H
#define TECH_READER_H

#include <string>
#include <vector>

namespace tk
{

class Reader
{
public:
  static std::vector<char> ReadShader(const std::string &filename);
};

} // namespace tk

#endif // TECH_READER_H
