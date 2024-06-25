#include "core/logger.h"
#ifndef JET_S_PLAYER_H

#include "core/types.h"
#include "update_system.h"

namespace jet
{

class SPlayer : public SUpdate
{
  i32 x;
  i32 y;

public:
  inline void Init() override
  {
    x = 100;
    y = 0;
  }

  inline void Update(f32 deltaTime) override
  {
    x += 1;
    y += 20 % x;
    Logger::Info("X:{}, Y:{}", x, y);
  }

  inline void Shutdown() override
  {
  }
};

} // namespace jet

#endif // !JET_S_PLAYER_H
