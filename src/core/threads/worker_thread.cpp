#include "worker_thread.h"
#include <thread>

namespace tk
{

WorkerThread::WorkerThread() : bPaused(false)
{
  mThread = new std::thread;
}

WorkerThread::~WorkerThread()
{
  delete mThread;
}

void WorkerThread::Run()
{
  bRunning = true;
  *mThread = std::thread([this]() {
    while (Running())
    {
      if (NotPaused())
      {
        mFunction();
      }
    }
  });
}

void WorkerThread::SetFunction(std::function<void()>&& func)
{
  mFunction = std::move(func);
}

void WorkerThread::SetId(i16 id)
{
  Id = id;
}

const std::function<void()>& WorkerThread::GetFunction() const
{
  return mFunction;
}

void WorkerThread::Pause()
{
  bPaused = true;
}

bool WorkerThread::Running()
{
  return bRunning;
}

bool WorkerThread::NotPaused()
{
  return !bPaused;
}

void WorkerThread::Join()
{
  bRunning = false;
  mThread->join();
}

} // namespace tk
