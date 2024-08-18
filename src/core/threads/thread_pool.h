#ifndef TK_JOBS_H
#define TK_JOBS_H

#include "core/dynamic_array.h"
#include "core/threads/worker_thread.h"

namespace tk
{

class ThreadPool
{
  DynamicArray<class WorkerThread*> mWorkers = {};
  u32 mNumWorkers = 0;

public:
  ThreadPool();
  void Run();
};

} // namespace tk

#endif // !TK_JOBS_H
