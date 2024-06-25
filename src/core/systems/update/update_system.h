#ifndef TECH_S_UPDATE_H
#define TECH_S_UPDATE_H

#include "../system.h"
#include "core/types.h"

namespace tk
{

class SUpdate : public System
{
public:
  virtual void Update(f32 deltaTime) = 0;
};

} // namespace tk

#endif // TECH_S_UPDATE_H
