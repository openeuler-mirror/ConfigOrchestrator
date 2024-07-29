#include <gtest/gtest.h>
#include <iostream>
#include <memory>

#include "backend/firewall/firewall_backend.h"
#include "tools/cplog.h"

using std::cout;
using std::endl;

class FirewallTest : public ::testing::Test {
protected:
  void SetUp() override { fwb = std::make_shared<FirewallBackend>(); }

  void TearDown() override {}

  std::shared_ptr<FirewallBackend> fwb;
};

void TestChainContext(const std::shared_ptr<FirewallBackend> &fwb,
                      const std::shared_ptr<FirewallContext> &tb_ctx,
                      const std::string &table, const std::string &chain) {
  auto ch_ctx = fwb->createContext(tb_ctx, chain);

  ASSERT_EQ(ch_ctx->level_, FirewallLevel::CHAIN);
  ASSERT_EQ(ch_ctx->table_, table);
  ASSERT_EQ(ch_ctx->chain_, chain);
  std::cout << "Table: " << table << "\tChain: " << chain << endl;

  auto rules = fwb->getSubconfigs(ch_ctx);
  for (const auto &rule : rules) {
    std::cout << rule << std::endl;
  }
}

void TestTableContext(const std::shared_ptr<FirewallBackend> &fwb,
                      const std::string &table) {
  auto ctx = std::make_shared<FirewallContext>();
  auto tb_ctx = fwb->createContext(ctx, table);

  ASSERT_EQ(tb_ctx->level_, FirewallLevel::TABLE);
  ASSERT_EQ(tb_ctx->table_, table);

  auto chains = fwb->getSubconfigs(tb_ctx);
  for (const auto &chain : chains) {
    TestChainContext(fwb, tb_ctx, table, chain);
  }
}

TEST_F(FirewallTest, get_chain) {
  auto ctx = std::make_shared<FirewallContext>();
  auto tables = fwb->getTableNames();
  for (const auto &table : tables) {
    TestTableContext(fwb, table);
  }
}
auto main(int argc, char **argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
