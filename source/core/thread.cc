#include "thread.h"

#include <future>

#include "platform.h"

#if P_OS_WINDOWS
#include <windows.h>
// The information on how to set the thread name comes from
// a MSDN article: http://msdn2.microsoft.com/en-us/library/xcb2z8hs.aspx
constexpr DWORD kVCThreadNameException = 0x406D1388;
typedef struct tagTHREADNAME_INFO {
  DWORD dwType;      // Must be 0x1000.
  LPCSTR szName;     // Pointer to name (in user addr space).
  DWORD dwThreadID;  // Thread ID (-1=caller thread).
  DWORD dwFlags;     // Reserved for future use, must be zero.
} THREADNAME_INFO;
#else
#include <pthread.h>
#endif

namespace pixel {

static void SetCurrentThreadName(const std::string& thread_name) {
#if P_OS_WINDOWS
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = thread_name.c_str();
  info.dwThreadID = GetCurrentThreadId();
  info.dwFlags = 0;
  __try {
    RaiseException(kVCThreadNameException, 0, sizeof(info) / sizeof(DWORD),
                   reinterpret_cast<DWORD_PTR*>(&info));
  } __except (EXCEPTION_CONTINUE_EXECUTION) {
  }
#elif P_OS_MAC
  pthread_setname_np(thread_name.c_str());
#elif P_OS_LINUX
  pthread_setname_np(pthread_self(), thread_name.c_str());
#else
#error Cannot set thread name on this platform.
#endif
}

void Thread::PostBackgroundTask(Closure closure) {
  if (!closure) {
    return;
  }
  static Thread sThread("BackgroundWorker");
  sThread.GetDispatcher()->PostTask(closure);
}

Thread::Thread(std::string thread_name) {
  std::promise<std::shared_ptr<EventLoop::Dispatcher>> on_done;
  auto on_done_future = on_done.get_future();
  thread_ =
      std::make_unique<std::thread>([thread_name = std::move(thread_name),
                                     on_done = std::move(on_done)]() mutable {
        SetCurrentThreadName(std::move(thread_name));
        auto& loop = EventLoop::ForCurrentThread();
        on_done.set_value(loop.GetDispatcher());
        loop.Run();
      });
  dispatcher_ = on_done_future.get();
}

Thread::~Thread() {
  thread_->join();
}

std::shared_ptr<EventLoop::Dispatcher> Thread::GetDispatcher() const {
  return dispatcher_;
}

void Thread::Terminate() {
  dispatcher_->PostTask([]() { EventLoop::ForCurrentThread().Terminate(); });
}

}  // namespace pixel
