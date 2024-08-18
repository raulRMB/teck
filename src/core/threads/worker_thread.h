#ifndef TK_WORKER_THREAD_H
#define TK_WORKER_THREAD_H

#include "core/dynamic_array.h"
#include <functional>

enum class EWorkerThreadType : u8
{
  Undefined = 0,
  Render,
  GameWrite,
  GameRead,
  General,
};

namespace std
{

class thread;

}

namespace tk
{

class WorkerThread
{
  u8 bRunning : 1;
  u8 bPaused : 1;

  std::function<void()> mFunction;
  EWorkerThreadType mThreadType = EWorkerThreadType::Undefined;
  class std::thread* mThread;
  i16 Id = -1;

public:
  WorkerThread();
  ~WorkerThread();

  void Run();
  void SetFunction(std::function<void()>&& function);
  const std::function<void()>& GetFunction() const;
  void SetId(i16 Id);
  void Pause();
  bool Running();
  bool NotPaused();
  void Join();
};

} // namespace tk

#endif //! TK_WORKER_THREAD_H
