#include <gtest/gtest.h>
#include <memory>

#include "backend/package_manager_backend.h"

class PackageManagerTest : public ::testing::Test {
protected:
  void SetUp() override { fwb = std::make_shared<PackageManagerBackend>(); }

  void TearDown() override {}

  std::shared_ptr<PackageManagerBackend> fwb;
};

TEST_F(PackageManagerTest, get_chain) {}

auto main(int argc, char **argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
