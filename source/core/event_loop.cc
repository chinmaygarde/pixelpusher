#include "event_loop.h"
#include <mutex>
#include <thread>

#include "logging.h"
#include "macros.h"

namespace pixel {

thread_local std::unique_ptr<EventLoop> tEventLoop;

struct TasksHeap {
  std::condition_variable tasks_cv;
  std::mutex tasks_mutex;
  std::vector<Closure> tasks;

  bool HasTasksLocked() const { return tasks.size() > 0; }

  std::vector<Closure> GetPendingTasksLocked() {
    auto copied_tasks = std::move(tasks);
    tasks.clear();
    return copied_tasks;
  }

  bool PostTaskLocked(Closure task) {
    if (!task) {
      return false;
    }
    tasks.push_back(task);

    // TODO: Ideally, this must be outside the critical section.
    tasks_cv.notify_one();
    return true;
  }
};

EventLoop& EventLoop::ForCurrentThread() {
  if (!tEventLoop) {
    tEventLoop = std::unique_ptr<EventLoop>(new EventLoop());
  }
  return *tEventLoop;
}

std::shared_ptr<EventLoop::Dispatcher> EventLoop::GetCurrentThreadDispatcher() {
  return ForCurrentThread().GetDispatcher();
}

EventLoop::EventLoop()
    : thread_id_(std::this_thread::get_id()),
      tasks_heap_(std::make_shared<TasksHeap>()),
      dispatcher_(std::shared_ptr<Dispatcher>(
          new Dispatcher(tasks_heap_, std::this_thread::get_id()))) {}

EventLoop::~EventLoop() = default;

bool EventLoop::IsRunning() const {
  return running_;
}

bool EventLoop::Run() {
  if (thread_id_ != std::this_thread::get_id()) {
    P_ASSERT(false);
    return false;
  }

  if (running_) {
    return true;
  }

  running_ = true;

  while (running_) {
    std::unique_lock lock(tasks_heap_->tasks_mutex);

    tasks_heap_->tasks_cv.wait(
        lock, [&]() { return tasks_heap_->HasTasksLocked() && running_; });

    auto tasks = tasks_heap_->GetPendingTasksLocked();

    lock.unlock();

    for (const auto& task : tasks) {
      task();
    }
  }

  return true;
}

bool EventLoop::FlushTasksNow() {
  if (thread_id_ != std::this_thread::get_id()) {
    P_ASSERT(false);
    return false;
  }

  std::unique_lock lock(tasks_heap_->tasks_mutex);

  auto tasks = tasks_heap_->GetPendingTasksLocked();

  lock.unlock();

  for (const auto& task : tasks) {
    task();
  }

  return true;
}

std::shared_ptr<EventLoop::Dispatcher> EventLoop::GetDispatcher() const {
  return dispatcher_;
}

EventLoop::Dispatcher::Dispatcher(std::weak_ptr<TasksHeap> heap,
                                  std::thread::id thread_id)
    : tasks_heap_(std::move(heap)), thread_id_(thread_id) {}

EventLoop::Dispatcher::~Dispatcher() = default;

bool EventLoop::Dispatcher::PostTask(Closure closure) {
  if (!closure) {
    return false;
  }

  if (auto tasks_heap = tasks_heap_.lock()) {
    std::scoped_lock lock(tasks_heap->tasks_mutex);
    return tasks_heap->PostTaskLocked(std::move(closure));
  }

  return false;
};

bool EventLoop::Dispatcher::RunsTasksOnCurrentThread() const {
  return std::this_thread::get_id() == thread_id_;
}

bool EventLoop::Terminate() {
  if (thread_id_ != std::this_thread::get_id()) {
    P_ASSERT(false);
    return false;
  }
  running_ = false;
  tasks_heap_->tasks_cv.notify_all();
  return true;
}

}  // namespace pixel
