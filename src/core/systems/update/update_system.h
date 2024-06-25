#ifndef JET_S_UPDATE_H
#define JET_S_UPDATE_H

#include "../system.h"
#include "core/types.h"

namespace jet
{

class SUpdate : public System
{
public:
  virtual void Update(f32 deltaTime) = 0;
};

} // namespace jet

#endif // JET_S_UPDATE_H
