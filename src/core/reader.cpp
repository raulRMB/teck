#include "reader.h"
#include "core/logger.h"
#include <fstream>

namespace tk
{

std::vector<char> Reader::ReadShader(const std::string &filename)
{
#ifdef _WIN32
  std::string shaderAddress = "../rec/shaders/" + filename + ".spv";
#else
  std::string shaderAddress = "rec/shaders/" + filename + ".spv";
#endif
  std::ifstream file(shaderAddress, std::ios::ate | std::ios::binary);

  if (!file.is_open())
  {
    Logger::Error("Failed to open {}", filename);
    throw std::runtime_error("");
  }

  size_t fileSize = (size_t)file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}

} // namespace tk
