#ifndef TECH_SYSTEM_H
#define TECH_SYSTEM_H

namespace tk
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

} // namespace tk

#endif // TECH_SYSTEM_H
