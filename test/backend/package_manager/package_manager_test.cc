#include <gtest/gtest.h>
#include <memory>

#include "backend/package_manager/package_manager_backend.h"

class PackageManagerTest : public ::testing::Test {
protected:
  void SetUp() override { pkb = std::make_shared<PackageManagerBackend>(); }

  void TearDown() override {}

  std::shared_ptr<PackageManagerBackend> pkb;
};

TEST_F(PackageManagerTest, get_packages) { ASSERT_EQ(0, 0); }

auto main(int argc, char **argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
