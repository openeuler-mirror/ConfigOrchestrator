#include <gtest/gtest-param-test.h>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#include "backend/firewall/chain_request.h"
#include "backend/firewall/firewall_backend.h"
#include "backend/firewall/firewall_context.h"
#include "backend/firewall/rule_request.h"
#include "tools/log.h"
#include "tools/nettools.h"

using std::cout;
using std::endl;
using std::make_optional;
using std::make_shared;
using std::make_tuple;
using std::nullopt;

class FirewallTestFixture : public ::testing::Test {
protected:
  void SetUp() override { fwb = make_shared<FirewallBackend>(); }

  void TearDown() override {}

  shared_ptr<FirewallBackend> fwb;
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

void TestChainAddDel(const shared_ptr<FirewallBackend> &fwb,
                     const string &table) {
  auto ctx = make_shared<FirewallContext>();
  ctx = fwb->createContext(ctx, table);

  ASSERT_EQ(ctx->level_, FirewallLevel::TABLE);
  ASSERT_EQ(ctx->table_, table);

  auto chains = fwb->getFirewallChildren(ctx);
  for (const auto &chain : chains) {
    auto ch_ctx = fwb->createContext(ctx, chain);

    ASSERT_EQ(ch_ctx->level_, FirewallLevel::CHAIN);
    ASSERT_EQ(ch_ctx->table_, table);
    ASSERT_EQ(ch_ctx->chain_, chain);

    ASSERT_FALSE(fwb->insertChain(ctx, make_shared<ChainRequest>(chain)));
  }

  auto new_chains = fwb->getFirewallChildren(ctx);
  ASSERT_EQ(chains.size(), new_chains.size());

  ASSERT_TRUE(fwb->insertChain(ctx, make_shared<ChainRequest>("NEW_CHAIN")));
  auto ch_ctx = fwb->createContext(ctx, "NEW_CHAIN");
  ASSERT_EQ(ch_ctx->level_, FirewallLevel::CHAIN);
  ASSERT_EQ(ch_ctx->table_, table);
  ASSERT_EQ(ch_ctx->chain_, "NEW_CHAIN");
  new_chains = fwb->getFirewallChildren(ctx);
  ASSERT_EQ(chains.size() + 1, new_chains.size());

  auto commit = fwb->apply();
  ASSERT_TRUE(commit());

  {
    auto new_fwb = make_shared<FirewallBackend>();
    auto new_ctx = make_shared<FirewallContext>();
    new_ctx = new_fwb->createContext(new_ctx, table);
    new_chains = new_fwb->getFirewallChildren(new_ctx);
    ASSERT_EQ(chains.size() + 1, new_chains.size());

    new_ctx = new_fwb->createContext(new_ctx, "NEW_CHAIN");
    ASSERT_EQ(new_ctx->level_, FirewallLevel::CHAIN);
    ASSERT_EQ(new_ctx->table_, table);
    ASSERT_EQ(new_ctx->chain_, "NEW_CHAIN");
  }

  ASSERT_TRUE(fwb->removeChain(ch_ctx));
  new_chains = fwb->getFirewallChildren(ctx);
  ASSERT_EQ(chains.size(), new_chains.size());

  ASSERT_TRUE(commit());
  {
    auto new_fwb = make_shared<FirewallBackend>();
    auto new_ctx = make_shared<FirewallContext>();
    new_ctx = new_fwb->createContext(new_ctx, table);
    new_chains = new_fwb->getFirewallChildren(new_ctx);
    ASSERT_EQ(chains.size(), new_chains.size());
  }
}

TEST_F(FirewallTestFixture, getChain) {
  auto ctx = make_shared<FirewallContext>();
  auto tables = fwb->getTableNames();
  for (const auto &table : tables) {
    TestTableContext(fwb, table);
  }
}

TEST_F(FirewallTestFixture, addDelChain) {
  auto ctx = make_shared<FirewallContext>();
  auto tables = fwb->getTableNames();
  for (const auto &table : tables) {
    TestChainAddDel(fwb, table);
  }
}

/* gtest param test for add/delete rule */
struct FirewallTestAddDelRuleData {
  FirewallTestAddDelRuleData(string a, string b,
                             std::shared_ptr<RuleRequest> req)
      : table(a), chain(b), rule_request(req) {}

  string table;
  string chain;

  shared_ptr<RuleRequest> rule_request;
};

class FirewallTestAddDelRule
    : public ::testing::TestWithParam<FirewallTestAddDelRuleData> {
protected:
  void SetUp() override {
    fwb = make_shared<FirewallBackend>();
    gen.seed(42);
  }

  void TearDown() override {}

  shared_ptr<FirewallBackend> fwb;
  std::mt19937 gen; // generate insert position
};

TEST_P(FirewallTestAddDelRule, AddThenDelRule) {
  const auto &param = GetParam();

  auto context = make_shared<FirewallContext>();
  context = fwb->createContext(context, param.table);
  context = fwb->createContext(context, param.chain);

  ASSERT_EQ(context->level_, FirewallLevel::CHAIN);
  ASSERT_EQ(context->table_, param.table);
  ASSERT_EQ(context->chain_, param.chain);

  auto rules = fwb->getFirewallChildren(context);
  auto rule_num = static_cast<int>(rules.size());

  // generate a random position
  std::uniform_int_distribution<> dis(0, rule_num);
  auto pos = dis(gen);
  param.rule_request->index_ = pos;

  ASSERT_TRUE(fwb->insertRule(context, param.rule_request));

  rules = fwb->getFirewallChildren(context);
  ASSERT_EQ(rule_num + 1, static_cast<int>(rules.size()));

  for (int i = 0; i <= rule_num; i++) {
    auto rule = fwb->getRule(context, i);
    ASSERT_NE(rule, nullptr);
    ASSERT_EQ(rule->index_, i);

    if (i == param.rule_request->index_) {
      ASSERT_EQ(rule->proto_, param.rule_request->proto_);
      ASSERT_EQ(rule->target_, param.rule_request->target_);

      ASSERT_TRUE(rule->src_mask_.has_value());
      ASSERT_TRUE(rule->dst_mask_.has_value());
      ASSERT_TRUE(rule->src_ip_.has_value());
      ASSERT_TRUE(rule->dst_ip_.has_value());

      if (param.rule_request->src_ip_.has_value()) {
        ASSERT_EQ(rule->src_ip_.value(), param.rule_request->src_ip_.value());
      } else {
        ASSERT_EQ(rule->src_ip_.value(), "0.0.0.0");
      }
      if (param.rule_request->dst_ip_.has_value()) {
        ASSERT_EQ(rule->dst_ip_.value(), param.rule_request->dst_ip_.value());
      } else {
        ASSERT_EQ(rule->dst_ip_.value(), "0.0.0.0");
      }

      if (param.rule_request->src_mask_.has_value()) {
        ASSERT_EQ(rule->src_mask_.value(),
                  param.rule_request->src_mask_.value());
      } else {
        ASSERT_EQ(rule->src_mask_.value(), "255.255.255.255");
      }

      if (param.rule_request->dst_mask_.has_value()) {
        ASSERT_EQ(rule->dst_mask_.value(),
                  param.rule_request->dst_mask_.value());
      } else {
        ASSERT_EQ(rule->dst_mask_.value(), "255.255.255.255");
      }

      ASSERT_EQ(rule->matches_.size(), param.rule_request->matches_.size());
      if (rule->matches_.size() > 0) {
        ASSERT_TRUE(rule->matches_[0].dst_port_range_.has_value());
        ASSERT_TRUE(rule->matches_[0].src_port_range_.has_value());

        if (param.rule_request->matches_[0].src_port_range_.has_value()) {
          ASSERT_EQ(rule->matches_[0].src_port_range_.value(),
                    param.rule_request->matches_[0].src_port_range_.value());
        } else {
          ASSERT_EQ(rule->matches_[0].src_port_range_.value(),
                    make_tuple("0", "65535"));
        }

        if (param.rule_request->matches_[0].dst_port_range_.has_value()) {
          ASSERT_EQ(rule->matches_[0].dst_port_range_.value(),
                    param.rule_request->matches_[0].dst_port_range_.value());
        } else {
          ASSERT_EQ(rule->matches_[0].dst_port_range_.value(),
                    make_tuple("0", "65535"));
        }
      }

      ASSERT_EQ(rule->iniface_.has_value(),
                param.rule_request->iniface_.has_value());
      if (rule->iniface_.has_value()) {
        ASSERT_EQ(rule->iniface_.value(), param.rule_request->iniface_.value());
      }
      ASSERT_EQ(rule->outiface_.has_value(),
                param.rule_request->outiface_.has_value());
      if (rule->outiface_.has_value()) {
        ASSERT_EQ(rule->outiface_.value(),
                  param.rule_request->outiface_.value());
      }
    }
  }

  auto commit = fwb->apply();
  ASSERT_TRUE(commit());

  // after commit, the rule should be saved
  {
    auto new_fwb = make_shared<FirewallBackend>();
    rules = new_fwb->getFirewallChildren(context);
    ASSERT_EQ(rule_num + 1, static_cast<int>(rules.size()));

    for (int i = 0; i <= rule_num; i++) {
      auto rule = new_fwb->getRule(context, i);
      ASSERT_NE(rule, nullptr);
      ASSERT_EQ(rule->index_, i);

      if (i == param.rule_request->index_) {
        ASSERT_EQ(rule->proto_, param.rule_request->proto_);
        ASSERT_EQ(rule->target_, param.rule_request->target_);

        ASSERT_TRUE(rule->src_mask_.has_value());
        ASSERT_TRUE(rule->dst_mask_.has_value());
        ASSERT_TRUE(rule->src_ip_.has_value());
        ASSERT_TRUE(rule->dst_ip_.has_value());

        if (param.rule_request->src_ip_.has_value()) {
          ASSERT_EQ(rule->src_ip_.value(), param.rule_request->src_ip_.value());
        } else {
          ASSERT_EQ(rule->src_ip_.value(), "0.0.0.0");
        }
        if (param.rule_request->dst_ip_.has_value()) {
          ASSERT_EQ(rule->dst_ip_.value(), param.rule_request->dst_ip_.value());
        } else {
          ASSERT_EQ(rule->dst_ip_.value(), "0.0.0.0");
        }

        if (param.rule_request->src_mask_.has_value()) {
          ASSERT_EQ(rule->src_mask_.value(),
                    param.rule_request->src_mask_.value());
        } else {
          ASSERT_EQ(rule->src_mask_.value(), "255.255.255.255");
        }

        if (param.rule_request->dst_mask_.has_value()) {
          ASSERT_EQ(rule->dst_mask_.value(),
                    param.rule_request->dst_mask_.value());
        } else {
          ASSERT_EQ(rule->dst_mask_.value(), "255.255.255.255");
        }

        ASSERT_EQ(rule->matches_.size(), param.rule_request->matches_.size());
        if (!rule->matches_.empty()) {
          ASSERT_TRUE(rule->matches_[0].dst_port_range_.has_value());
          ASSERT_TRUE(rule->matches_[0].src_port_range_.has_value());

          if (param.rule_request->matches_[0].src_port_range_.has_value()) {
            ASSERT_EQ(rule->matches_[0].src_port_range_.value(),
                      param.rule_request->matches_[0].src_port_range_.value());
          } else {
            ASSERT_EQ(rule->matches_[0].src_port_range_.value(),
                      make_tuple("0", "65535"));
          }

          if (param.rule_request->matches_[0].dst_port_range_.has_value()) {
            ASSERT_EQ(rule->matches_[0].dst_port_range_.value(),
                      param.rule_request->matches_[0].dst_port_range_.value());
          } else {
            ASSERT_EQ(rule->matches_[0].dst_port_range_.value(),
                      make_tuple("0", "65535"));
          }
        }

        ASSERT_EQ(rule->iniface_.has_value(),
                  param.rule_request->iniface_.has_value());
        if (rule->iniface_.has_value()) {
          ASSERT_EQ(rule->iniface_.value(),
                    param.rule_request->iniface_.value());
        }
        ASSERT_EQ(rule->outiface_.has_value(),
                  param.rule_request->outiface_.has_value());
        if (rule->outiface_.has_value()) {
          ASSERT_EQ(rule->outiface_.value(),
                    param.rule_request->outiface_.value());
        }
      }
    }
  }

  // remove the rule
  ASSERT_TRUE(fwb->removeRule(context, pos));

  rules = fwb->getFirewallChildren(context);
  ASSERT_EQ(rule_num, static_cast<int>(rules.size()));
  for (int i = 0; i < rule_num; i++) {
    auto rule = fwb->getRule(context, i);
    ASSERT_NE(rule, nullptr);
    ASSERT_EQ(rule->index_, i);
  }

  ASSERT_TRUE(commit());
  {
    auto new_fwb = make_shared<FirewallBackend>();
    rules = new_fwb->getFirewallChildren(context);
    ASSERT_EQ(rule_num, static_cast<int>(rules.size()));

    for (int i = 0; i < rule_num; i++) {
      auto rule = new_fwb->getRule(context, i);
      ASSERT_NE(rule, nullptr);
      ASSERT_EQ(rule->index_, i);
    }
  }
}

auto GenerateRandomTestData(size_t count)
    -> vector<FirewallTestAddDelRuleData> {
  constexpr int RANDOM_SEED = 42;

  vector<FirewallTestAddDelRuleData> data;
  std::mt19937 gen{RANDOM_SEED};

  static const auto tables = FirewallBackend::getTableNames();
  static const auto targets = iptTargets();
  static const auto protos = protocols();

  static const vector<string> chains = {"INPUT", "OUTPUT"};

  for (size_t i = 0; i < count; ++i) {
    std::uniform_int_distribution<int> dist(1, 2);
  }

  return data;
}

static const int testDFirewallTestAddDelRuleInstantDataSize = 1000;
std::vector<FirewallTestAddDelRuleData> testDFirewallTestAddDelRuleInstantData =
    GenerateRandomTestData(testDFirewallTestAddDelRuleInstantDataSize);

INSTANTIATE_TEST_CASE_P(
    FirewallTestAddDelRuleFuzzingInstant, FirewallTestAddDelRule,
    ::testing::ValuesIn(testDFirewallTestAddDelRuleInstantData));

INSTANTIATE_TEST_CASE_P(
    FirewallTestAddDelRuleInstant, FirewallTestAddDelRule,
    ::testing::Values(
        FirewallTestAddDelRuleData(
            "filter", "INPUT",
            make_shared<RuleRequest>(
                0, make_optional<string>("1.2.3.4"),
                make_optional<string>("255.255.255.1"), nullopt, nullopt,
                RequestProto::UDP, make_optional<string>("eth1"), nullopt,
                vector<RuleMatch>{
                    {nullopt, make_optional(make_tuple("12", "145"))}},
                "ACCEPT")),
        FirewallTestAddDelRuleData(
            "filter", "INPUT",
            make_shared<RuleRequest>(
                0, make_optional<string>("89.31.112.2"),
                make_optional<string>("255.255.255.1"),
                make_optional<string>("89.31.112.2"),
                make_optional<string>("255.255.255.1"), RequestProto::UDP,
                make_optional<string>("eth1"), make_optional<string>("eth100"),
                vector<RuleMatch>{{make_optional(make_tuple("127", "1405")),
                                   make_optional(make_tuple("12", "145"))}},
                "DROP"))));

auto main(int argc, char **argv) -> int {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
