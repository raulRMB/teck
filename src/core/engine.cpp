#include "engine.h"

#include "logger.h"
#include "systems/update/SPlayer.h"
#include "systems/update/update_system.h"
#include <cstdlib>
#include <cstring>
#include <iostream>

namespace tk
{

Engine::Engine() : bRunning(false)
{
}

void Engine::Init()
{
  CHECK_IN();

  mWindow = Window(400, 400, "Jet");
  mWindow.Init();
  mRenderer.Init(&mWindow);
  bRunning = true;

  InitSystems();
}

void Engine::InitSystems()
{
  /*SPlayer *player = new SPlayer();*/
  /*player->Init();*/
  /*mUpdateSystems.emplace_back(player);*/
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

i32 Engine::Run(i32 argc, char **argv)
{
  CHECK_IN();

  Engine engine;

  engine.ParseArgs(argc, argv);

  try
  {
    engine.Init();

    do
    {
      engine.PollEvents();
      engine.mRenderer.ImGuiDraw();
      engine.mRenderer.DrawFrame();
      engine.Loop();
    } while (engine.bRunning);
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  engine.Clean();

  return 0;
}

void Engine::ParseArgs(i32 argc, char **argv)
{
  Logger::Info("Args:");
  for (i32 i = 1; i < argc; i++)
  {
    Logger::Message("{}: {}", i, argv[i]);
  }

  if (argc > 1)
  {
    bIsServer = false;
    /*if (std::string(argv[1]) == "c")*/
    /*{*/
    /*  bIsServer = false;*/
    /*}*/
    /*else if (std::string(argv[1]) == "s")*/
    /*{*/
    /*  bIsServer = true;*/
    /*}*/
  }
  else
  {
    bIsServer = true;
  }
}

void Engine::PollEvents()
{
  if (mWindow.ShouldClose())
  {
    bRunning = false;
  }

  mWindow.PollEvents();
}

void Engine::Loop()
{
  for (SUpdate *system : mUpdateSystems)
  {
    system->Update(0.f);
  }
}

void Engine::Clean()
{
  CHECK_IN();

  CleanSystems();
  mRenderer.Clean();
  mWindow.Clean();
}

} // namespace tk
