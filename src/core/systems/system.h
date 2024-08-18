#ifndef TECH_SYSTEM_H
#define TECH_SYSTEM_H

#include "core/ecs/registry.h"

namespace tk
{

class System
{
protected:
  static Registry& GetRegistry();
  static entt::entity CreateEntity()
  {
    return GetRegistry().create();
  }
  template <typename C> static C& GetComponent(const entt::entity& entity)
  {
    return GetRegistry().get<C>(entity);
  }
  template <typename C> static void AddComponent(const entt::entity& entity, C& Component)
  {
    GetRegistry().emplace<C>(entity, Component);
  }
  template <typename... C> static auto& GetView()
  {
    return GetRegistry().view<C...>();
  }

public:
  virtual void Init() = 0;
  virtual void Shutdown() = 0;
  virtual ~System() = default;
};

} // namespace tk

#endif // TECH_SYSTEM_H
