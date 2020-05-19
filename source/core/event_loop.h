#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <thread>
#include <vector>

#include "closure.h"
#include "macros.h"

namespace pixel {

struct TasksHeap;

class EventLoop {
 public:
  class Dispatcher {
   public:
    bool PostTask(Closure closure);

    bool RunsTasksOnCurrentThread() const;

    ~Dispatcher();

   private:
    friend class EventLoop;

    std::weak_ptr<TasksHeap> tasks_heap_;
    const std::thread::id thread_id_;

    Dispatcher(std::weak_ptr<TasksHeap> heap, std::thread::id thread_id);

    P_DISALLOW_COPY_AND_ASSIGN(Dispatcher);
  };

  static EventLoop& ForCurrentThread();

  ~EventLoop();

  bool IsRunning() const;

  std::shared_ptr<Dispatcher> GetDispatcher() const;

  bool Run();

  bool FlushTasksNow();

  bool Terminate();

 private:
  std::thread::id thread_id_;
  std::atomic_bool running_ = false;
  std::shared_ptr<TasksHeap> tasks_heap_;
  std::shared_ptr<Dispatcher> dispatcher_;

  EventLoop();

  P_DISALLOW_COPY_AND_ASSIGN(EventLoop);
};

}  // namespace pixel
