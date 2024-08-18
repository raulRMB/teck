#include "client_engine.h"
#include "core/logger.h"
#include "core/renderer.h"
#include "core/window.h"

namespace tk
{

ClientEngine::ClientEngine()
{
}

void ClientEngine::Init()
{
  bRunning = true;
  mWindow = new Window(400, 400, "tk");
  mWindow->Init();
  mRenderer = new Renderer();
  mRenderer->Init(mWindow);
}

i32 ClientEngine::Run(i32 argc, char** argv)
{
  CHECK_IN();

  ClientEngine engine;

  engine.ParseArgs(argc, argv);

  try
  {
    engine.Init();

    while (engine.bRunning)
    {
      engine.PollEvents();
      engine.Loop();
      engine.Draw();
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  engine.Clean();

  return 0;
}

void ClientEngine::PollEvents()
{
  if (mWindow->ShouldClose())
  {
    bRunning = false;
  }
  mWindow->PollEvents();
}

void ClientEngine::Draw()
{
  mRenderer->ImGuiDraw();
  mRenderer->DrawFrame();
}

void ClientEngine::Clean()
{
  mRenderer->Clean();
  delete mRenderer;

  mWindow->Clean();
  delete mWindow;
}

} // namespace tk
