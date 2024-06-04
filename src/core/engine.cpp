#include "engine.h"

#include <iostream>

#include "logger.h"

namespace jet
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
}

i32 Engine::Run()
{
  CHECK_IN();

  Engine engine;

  try
  {
    engine.Init();

    while (engine.bRunning)
    {
      engine.PollEvents();
      engine.mRenderer.DrawFrame();
      engine.Loop();
    };
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  engine.Clean();

  return 0;
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
}

void Engine::Clean()
{
  CHECK_IN();

  mRenderer.Clean();
  mWindow.Clean();
}

} // namespace jet
