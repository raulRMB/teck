#ifndef JET_READER_H
#define JET_READER_H

#include <string>
#include <vector>

namespace jet
{

class Reader
{
public:
  static std::vector<char> ReadShader(const std::string &filename);
};

} // namespace jet

#endif // JET_READER_H
