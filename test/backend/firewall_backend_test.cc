#include <gtest/gtest.h>
#include <memory>

#include "backend/firewall_backend.h"

class FirewallTest : public ::testing::Test {
protected:
  void SetUp() override { fwb = std::make_shared<FirewallBackend>(); }

  void TearDown() override {}

  std::shared_ptr<FirewallBackend> fwb;
};

TEST_F(FirewallTest, get_chain) {}

auto main(int argc, char **argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
