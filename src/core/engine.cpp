#include "engine.h"
#include <iostream>

namespace jet
{

Engine::Engine() :
  bRunning(false)
{}

void Engine::Init()
{
  mWindow = Window(400, 400, "Jet");
  mWindow.Init();
  mRenderer.vCreateInstance();  
  bRunning = true;
}

i32 Engine::Run()
{
  Engine engine;
  
  try 
  {
    engine.Init();
  
    while(engine.bRunning) 
    {
      engine.PollEvents(); 
      engine.Loop();
    };  
  } 
  catch (const std::exception& e) 
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  engine.Clean();
  
  return 0;
}

void Engine::PollEvents()
{
  if(mWindow.ShouldClose())
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
}

}
