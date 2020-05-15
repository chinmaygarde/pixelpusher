
#include <mutex>
#include <sstream>

#include <gtest/gtest.h>

#include "event_loop.h"

namespace pixel {
namespace testing {

TEST(EventLoopTest, CanAcquireThreadLocalEventLoop) {
  auto& loop = EventLoop::ForCurrentThread();
  ASSERT_FALSE(loop.IsRunning());
};

TEST(EventLoopTest, CanRunAndPostTasks) {
  std::mutex concat_lock;
  std::stringstream stream;

  auto locked_concat = [&](std::string string) {
    std::scoped_lock lock(concat_lock);
    stream << string;
  };

  std::thread thread([&]() {
    auto& loop = EventLoop::ForCurrentThread();
    ASSERT_FALSE(loop.IsRunning());
    auto dispatcher = loop.GetDispatcher();
    ASSERT_TRUE(dispatcher);

    dispatcher->PostTask([&]() { locked_concat("a"); });
    dispatcher->PostTask([&]() { locked_concat("b"); });
    dispatcher->PostTask([&]() { locked_concat("c"); });
    dispatcher->PostTask([&]() { locked_concat("d"); });
    dispatcher->PostTask([&]() { locked_concat("e"); });
    dispatcher->PostTask([&]() {
      auto& this_loop = EventLoop::ForCurrentThread();
      ASSERT_EQ(&this_loop, &loop);
      this_loop.Terminate();
    });

    ASSERT_TRUE(loop.Run());
  });

  thread.join();

  ASSERT_EQ(stream.str(), "abcde");
};

}  // namespace testing
}  // namespace pixel
