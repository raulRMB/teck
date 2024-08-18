#include "engine.h"

#include "core/renderer.h"
#include "core/window.h"
#include "logger.h"
#include "systems/update/s_shape.h"
#include "systems/update/update_system.h"
#include <cstring>

namespace tk
{

Engine::Engine() : bRunning(false)
{
}

Engine& Engine::Get()
{
  return mInstance;
}

void Engine::Init()
{
  CHECK_IN();

  bRunning = true;

  InitSystems();
}

void Engine::InitSystems()
{
  mUpdateSystems.emplace_back(SShape());
}

void Engine::CleanSystems()
{
  for (size_t i; i < mUpdateSystems.size(); i++)
  {
    mUpdateSystems[i]->Shutdown();
    delete mUpdateSystems[i];
  }
  mUpdateSystems.clear();
}

void Engine::Draw()
{
}

void Engine::ParseArgs(i32 argc, char** argv)
{
  Logger::Info("Args:");
  for (i32 i = 1; i < argc; i++)
  {
    Logger::Message("{}: {}", i, argv[i]);
  }
}

void Engine::PollEvents()
{
}

void Engine::Loop()
{
  for (SUpdate* system : mUpdateSystems)
  {
    system->Update(0.f);
  }
}

void Engine::Clean()
{
  CHECK_IN();

  CleanSystems();
}

} // namespace tk
