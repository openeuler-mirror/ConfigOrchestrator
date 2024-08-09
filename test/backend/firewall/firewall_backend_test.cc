#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <tuple>

#include "backend/firewall/firewall_backend.h"
#include "backend/firewall/firewall_context.h"
#include "tools/log.h"

using std::cout;
using std::endl;
using std::make_optional;
using std::make_shared;
using std::make_tuple;
using std::nullopt;

class FirewallTest : public ::testing::Test {
protected:
  void SetUp() override {
    fwb = make_shared<FirewallBackend>();
    write_context = make_shared<FirewallContext>();
    write_context = fwb->createContext(write_context, write_table);
    write_context = fwb->createContext(write_context, write_chain);

    ASSERT_EQ(write_context->level_, FirewallLevel::CHAIN);
    ASSERT_EQ(write_context->table_, write_table);
    ASSERT_EQ(write_context->chain_, write_chain);

    auto rules = fwb->getFirewallChildren(write_context);
    rule_num = static_cast<int>(rules.size());
  }

  void TearDown() override {}

  const string write_table = "filter";
  const string write_chain = "INPUT";

  int rule_num;
  shared_ptr<FirewallBackend> fwb;
  shared_ptr<FirewallContext> write_context;
};

void TestChainContext(const shared_ptr<FirewallBackend> &fwb,
                      const shared_ptr<FirewallContext> &tb_ctx,
                      const string &table, const string &chain) {
  auto ch_ctx = fwb->createContext(tb_ctx, chain);

  ASSERT_EQ(ch_ctx->level_, FirewallLevel::CHAIN);
  ASSERT_EQ(ch_ctx->table_, table);
  ASSERT_EQ(ch_ctx->chain_, chain);
  cout << "Table: " << table << "\tChain: " << chain << endl;

  auto rules = fwb->getFirewallChildren(ch_ctx);
  for (const auto &rule : rules) {
    cout << rule << endl;
  }
}

void TestTableContext(const shared_ptr<FirewallBackend> &fwb,
                      const string &table) {
  auto ctx = make_shared<FirewallContext>();
  auto tb_ctx = fwb->createContext(ctx, table);

  ASSERT_EQ(tb_ctx->level_, FirewallLevel::TABLE);
  ASSERT_EQ(tb_ctx->table_, table);

  auto chains = fwb->getFirewallChildren(tb_ctx);
  for (const auto &chain : chains) {
    TestChainContext(fwb, tb_ctx, table, chain);
  }
}

TEST_F(FirewallTest, get_chain) {
  auto ctx = make_shared<FirewallContext>();
  auto tables = fwb->getTableNames();
  for (const auto &table : tables) {
    TestTableContext(fwb, table);
  }
}

TEST_F(FirewallTest, add_rule) {
  // insert a rule
  auto rule_request = make_shared<RuleRequest>(
      1, make_optional<string>("10.201.0.238"),
      make_optional<string>("255.255.255.255"), nullopt, nullopt,
      RequestProto::UDP, nullopt, nullopt,
      vector<RuleMatch>{
          RuleMatch{nullopt, optional<tuple<string, string>>(
                                 make_tuple<string, string>("22", "22"))}},
      "ACCEPT");

  ASSERT_TRUE(fwb->insertRule(write_context, rule_request));

  auto commit = fwb->apply();
  ASSERT_TRUE(commit());

  auto rules = fwb->getFirewallChildren(write_context);
  ASSERT_EQ(rule_num + 1, static_cast<int>(rules.size()));

  // remove the rule
  ASSERT_TRUE(fwb->removeRule(write_context, 0));
  ASSERT_TRUE(commit());

  rules = fwb->getFirewallChildren(write_context);
  ASSERT_EQ(rule_num, static_cast<int>(rules.size()));
}

auto main(int argc, char **argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
