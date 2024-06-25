#ifndef JET_SYSTEM_H
#define JET_SYSTEM_H

namespace jet
{

class System
{

public:
  virtual void Init() = 0;
  virtual void Shutdown() = 0;
  virtual ~System()
  {
  }
};

} // namespace jet

#endif // JET_SYSTEM_H
