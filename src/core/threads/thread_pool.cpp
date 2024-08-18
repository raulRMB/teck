#include "thread_pool.h"
#include "core/logger.h"
#include "core/threads/worker_thread.h"
#include <thread>

namespace tk
{

ThreadPool::ThreadPool() : mNumWorkers(std::thread::hardware_concurrency())
{
  if (mNumWorkers == 0)
  {
    mNumWorkers = 1;
  }
}

void ThreadPool::Run()
{
  for (u32 i = 0u; i < mNumWorkers; i++)
  {
    WorkerThread* thread = new WorkerThread();
    thread->SetFunction([i]() { Logger::Info("Id: {}", i); });
    thread->SetId(i);
    thread->Run();
    mWorkers.emplace_back(thread);
  }

  for (WorkerThread* worker : mWorkers)
  {
    worker->Join();
  }

  Logger::Info("All threads have completed their work");
}

} // namespace tk
