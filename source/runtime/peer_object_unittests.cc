#include <gtest/gtest.h>

#include "macros.h"
#include "peer_object.h"

namespace pixel {
namespace testing {

static size_t sCTypeConstructors = ~0;
static size_t sPeerTypeConstructors = ~0;
static size_t sCTypeDesctructors = ~0;
static size_t sPeerTypeDesctructors = ~0;

struct TestCType {
  void* ffi_peer;

  size_t baton = 42;

  TestCType() { sCTypeConstructors++; }

  ~TestCType() { sCTypeDesctructors++; }
};

class TestPeerObject : public PeerObject<TestCType> {
 public:
  TestPeerObject() { sPeerTypeConstructors++; }

  ~TestPeerObject() { sPeerTypeDesctructors++; }

 private:
  P_DISALLOW_COPY_AND_ASSIGN(TestPeerObject);
};

TEST(PeerObjectTest, CanCreate) {
  sCTypeConstructors = 0;
  sPeerTypeConstructors = 0;
  sCTypeDesctructors = 0;
  sPeerTypeDesctructors = 0;

  {
    auto object = std::make_shared<TestPeerObject>();
    ASSERT_TRUE(object->GetPeerObject().IsValid());
    ASSERT_EQ(sCTypeConstructors, 1u);
    ASSERT_EQ(sPeerTypeConstructors, 1u);
    ASSERT_EQ(sCTypeDesctructors, 0u);
    ASSERT_EQ(sPeerTypeDesctructors, 0u);

    ASSERT_EQ(object->GetPeerObject().Get()->Get()->baton, 42u);
  }

  ASSERT_EQ(sCTypeConstructors, 1u);
  ASSERT_EQ(sPeerTypeConstructors, 1u);
  ASSERT_EQ(sPeerTypeDesctructors, 1u);
  ASSERT_EQ(sCTypeDesctructors, 1u);
}

}  // namespace testing
}  // namespace pixel
