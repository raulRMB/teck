#include <stdexcept>

#define TRY_VK(fn, msg) \
  if (vk::Result result = fn; result != vk::Result::eSuccess) \
  { \
    throw std::runtime_error(msg); \
  }
