#pragma once

#include <string>
#include <thread>

#include "closure.h"
#include "event_loop.h"
#include "macros.h"

namespace pixel {

class Thread {
 public:
  static void PostBackgroundTask(Closure closure);

  Thread(std::string thread_name);

  ~Thread();

  std::shared_ptr<EventLoop::Dispatcher> GetDispatcher() const;

  void Terminate();

 private:
  std::unique_ptr<std::thread> thread_;
  std::shared_ptr<EventLoop::Dispatcher> dispatcher_;

  P_DISALLOW_COPY_AND_ASSIGN(Thread);
};

}  // namespace pixel
