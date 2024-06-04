#include "reader.h"
#include <fstream>

namespace jet
{

std::vector<char> Reader::ReadShader(const std::string &filename)
{
  std::string shaderAddress = "rec/shaders/" + filename + ".spv";
  std::ifstream file(shaderAddress, std::ios::ate | std::ios::binary);

  if (!file.is_open())
  {
    throw std::runtime_error("failed to open file!");
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

} // namespace jet
