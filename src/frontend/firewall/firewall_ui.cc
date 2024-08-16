#include "frontend/firewall/firewall_ui.h"
#include "backend/config_manager.h"
#include "backend/firewall/firewall_context.h"
#include "controlpanel.h"
#include "fmt/core.h"
#include "frontend/ui_base.h"
#include "tools/nettools.h"
#include "tools/uitools.h"
#include "tools/widget_manager.h"

#include <yui/YAlignment.h>
#include <yui/YButtonBox.h>
#include <yui/YCheckBox.h>
#include <yui/YCheckBoxFrame.h>
#include <yui/YComboBox.h>
#include <yui/YDialog.h>
#include <yui/YEmpty.h>
#include <yui/YEvent.h>
#include <yui/YFrame.h>
#include <yui/YImage.h>
#include <yui/YInputField.h>
#include <yui/YIntField.h>
#include <yui/YLabel.h>
#include <yui/YLayoutBox.h>
#include <yui/YLogView.h>
#include <yui/YMenuButton.h>
#include <yui/YMultiLineEdit.h>
#include <yui/YMultiSelectionBox.h>
#include <yui/YPackageSelector.h>
#include <yui/YProgressBar.h>
#include <yui/YPushButton.h>
#include <yui/YRadioButton.h>
#include <yui/YRadioButtonGroup.h>
#include <yui/YReplacePoint.h>
#include <yui/YRichText.h>
#include <yui/YSelectionBox.h>
#include <yui/YSpacing.h>
#include <yui/YSquash.h>
#include <yui/YTable.h>
#include <yui/YTableHeader.h>
#include <yui/YTimeField.h>
#include <yui/YTree.h>
#include <yui/YUI.h>
#include <yui/YWidget.h>
#include <yui/YWidgetFactory.h>

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using std::stringstream;

const string FirewallConfig::kNonSuWarnText =
    "Please run the control panel as root to configure firewall.";
const string FirewallConfig::kAddRuleButtonText = "&Add Firewall Rule";
const string FirewallConfig::kAddChainButtonText = "&Add Firewall Chain";
const string FirewallConfig::kDelRuleButtonText = "Delete";

FirewallConfig::FirewallConfig(const string &name,
                               const shared_ptr<UIBase> &parent,
                               ctx_t firewall_context)
    : UIBase(name, parent), firewall_context_(std::move(firewall_context)) {
  if (!isSuperUser()) {
    yuiError() << "Non-root user try to open firewall configuration." << endl;
    showDialog(dialog_meta::ERROR, kNonSuWarnText);
  }

  if (firewall_context_ == nullptr) {
    firewall_context_ = std::make_shared<FirewallContext>();
  }

  firewall_backend_ = ConfigManager::instance().getBackend<FirewallBackend>();
};

auto FirewallConfig::fresh(YDialog *main_dialog, DisplayLayout layout) -> bool {
  auto res = true;
  iptable_children = firewall_backend_->getFirewallChildren(firewall_context_);

  auto *fac = getFactory();
  auto *main_layout = layout.feature_layout_;

  widget_manager_.removeWidget(main_layout);

  main_layout->deleteChildren();
  switch (firewall_context_->level_) {
  case FirewallLevel::OVERALL: {
    for (const auto &child : iptable_children) {
      auto *chain_button = fac->createPushButton(main_layout, child);
      widget_manager_.addWidget(chain_button, [this, chain_button, child]() {
        auto context =
            firewall_backend_->createContext(firewall_context_, child);

        auto subpage = std::make_shared<FirewallConfig>(
            child, shared_from_this(), context);

        subpage->display();
        subpage->handleEvent();
        return HandleResult::SUCCESS;
      });
    }
    break;
  }
  case FirewallLevel::TABLE: {
    for (const auto &child : iptable_children) {
      auto *hbox = fac->createHBox(main_layout);

      auto *chain_button = fac->createPushButton(hbox, child);
      fac->createHSpacing(hbox, 2);
      auto *del_button = fac->createPushButton(hbox, "Delete");

      widget_manager_.addWidget(chain_button, [this, chain_button, child]() {
        auto context =
            firewall_backend_->createContext(firewall_context_, child);

        auto subpage = std::make_shared<FirewallConfig>(
            child, shared_from_this(), context);

        subpage->display();
        subpage->handleEvent();
        return HandleResult::SUCCESS;
      });
      widget_manager_.addWidget(
          del_button, [this, child, main_dialog, layout]() {
            auto remove_ctx =
                firewall_backend_->createContext(firewall_context_, child);

            if (!firewall_backend_->removeChain(remove_ctx)) {
              auto msg = fmt::format("Failed to remove chain: {}, Error: {}\n",
                                     child, remove_ctx->getLastError());
              showDialog(dialog_meta::ERROR, msg);
            } else {
              auto msg = fmt::format("Chain removed: {}\n", child);
              showDialog(dialog_meta::INFO, msg);

              ConfigManager::instance().registerApplyFunc(
                  firewall_backend_->apply());
              fresh(main_dialog, layout);
            }

            return HandleResult::SUCCESS;
          });
    }
    break;
  }
  case FirewallLevel::CHAIN: {
    for (int index = 0; index < iptable_children.size(); index++) {
      auto iptable_child = iptable_children[index];
      auto *hbox = fac->createHBox(main_layout);

      fac->createLabel(hbox, iptable_child);
      fac->createHSpacing(hbox, 2);
      auto *detail_button = fac->createPushButton(hbox, "Detail");
      fac->createHSpacing(hbox, 2);
      auto *del_button = fac->createPushButton(hbox, "Delete");
      fac->createHSpacing(hbox, 2);
      auto *update_button = fac->createPushButton(hbox, "Update");

      widget_manager_.addWidget(detail_button, [this, index]() {
        auto title = fmt::format("Rule Detail: #{}", index);
        showDialog(title,
                   firewall_backend_->getRuleDetails(firewall_context_, index));
        return HandleResult::SUCCESS;
      });

      widget_manager_.addWidget(del_button, [this, iptable_child, index,
                                             main_dialog, layout]() {
        auto res = firewall_backend_->removeRule(firewall_context_, index);

        if (!res) {
          auto msg = fmt::format(
              "Failed to remove chain: #{}\nChain brief: {}\nError: {}\n",
              index, iptable_child, firewall_context_->getLastError());
          showDialog(dialog_meta::ERROR, msg);
          return HandleResult::SUCCESS;
        }

        ConfigManager::instance().registerApplyFunc(firewall_backend_->apply());
        fresh(main_dialog, layout);
        return HandleResult::SUCCESS;
      });

      widget_manager_.addWidget(update_button, [this, iptable_child, index,
                                                main_dialog, layout]() {
        auto request = createUpdateRule(index);
        if (request == nullptr) {
          return HandleResult::SUCCESS; /* cancel */
        }

        if (!firewall_backend_->updateRule(firewall_context_, request)) {
          auto msg = fmt::format(
              "Failed to update chain: #{}\nChain brief: {}\nError: {}\n",
              index, iptable_child, firewall_context_->getLastError());

          showDialog(dialog_meta::ERROR, msg);
          return HandleResult::SUCCESS;
        }

        ConfigManager::instance().registerApplyFunc(firewall_backend_->apply());
        fresh(main_dialog, layout);
        return HandleResult::SUCCESS;
      });
    }
  }
  }

  // it's expensive for this operation
  main_dialog->recalcLayout();

  return res;
}

auto FirewallConfig::userDisplay(YDialog *main_dialog,
                                 DisplayLayout layout) -> DisplayResult {
  (void)main_dialog;

  auto *fac = getFactory();
  auto *control_layout = layout.user_control_layout_;

  /* control layout */
  switch (firewall_context_->level_) {
  case FirewallLevel::OVERALL: {
    break;
  }
  case FirewallLevel::TABLE: {
    auto *add_chain_button =
        fac->createPushButton(control_layout, kAddChainButtonText);

    widget_manager_.addWidget(add_chain_button, [this, main_dialog, layout]() {
      auto requset = createChain();
      if (requset == nullptr) {
        return HandleResult::SUCCESS; /* cancel */
      }

      if (!firewall_backend_->insertChain(firewall_context_, requset)) {
        auto msg = fmt::format("Failed to add chain, Error: {}\n",
                               firewall_context_->getLastError());
        showDialog(dialog_meta::ERROR, msg);
      } else {
        auto msg = fmt::format("Chain added: {}\n", requset->chain_name_);
        showDialog(dialog_meta::INFO, msg);

        ConfigManager::instance().registerApplyFunc(firewall_backend_->apply());
        fresh(main_dialog, layout);
      }

      return HandleResult::SUCCESS;
    });
    break;
  }

  case FirewallLevel::CHAIN: {
    auto *add_rule_button =
        fac->createPushButton(control_layout, kAddRuleButtonText);
    widget_manager_.addWidget(add_rule_button, [this, main_dialog, layout]() {
      auto request = createUpdateRule(std::nullopt);
      if (request == nullptr) {
        return HandleResult::SUCCESS;
      }

      if (!firewall_backend_->insertRule(firewall_context_, request)) {
        auto msg = fmt::format("Failed to add rule, Error: {}\n",
                               firewall_context_->getLastError());
        showDialog(dialog_meta::ERROR, msg);
        return HandleResult::SUCCESS;
      }

      ConfigManager::instance().registerApplyFunc(firewall_backend_->apply());
      fresh(main_dialog, layout);

      return HandleResult::SUCCESS;
    });
    break;
  }
  }

  /* main layout */
  fresh(main_dialog, layout);

  return DisplayResult::SUCCESS;
}

auto FirewallConfig::userHandleEvent(YEvent *event) -> HandleResult {
  return widget_manager_.handleEvent(event);
}

auto FirewallConfig::getPageDescription() const -> string {
  string componentDescription =
      fmt::format("Configure network firewall settings, {}\n",
                  firewall_context_->serialize());

  switch (firewall_context_->level_) {
  case FirewallLevel::OVERALL:
    componentDescription += "Select a table to check all chains and you are "
                            "allowed to add/delete chains.";
    break;
  case FirewallLevel::TABLE:
    componentDescription += "Select a chain to check all rules."
                            "You are allowed to add/delete rules.";
    break;
  case FirewallLevel::CHAIN:
    componentDescription += "You can see details of each rule and update or "
                            "delete them.";
    break;
  }
  return componentDescription;
};

auto FirewallConfig::getPageName() const -> string {
  static string componentName = "Network Firewall Configuration";

  if (firewall_context_->level_ == FirewallLevel::OVERALL) {
    return componentName;
  }
  return firewall_context_->serialize();
}

auto FirewallConfig::createUpdateRule(optional<int> index) // NOLINT
    -> shared_ptr<RuleRequest> {
  static constexpr int button_space = 5;

  auto *fac = getFactory();
  YDialog *dialog = fac->createPopupDialog();

  shared_ptr<RuleRequest> request;
  if (index.has_value()) { /* update dialog */
    request = firewall_backend_->getRule(firewall_context_, index.value());
  } else { /* insert dialog */
    request = std::make_shared<RuleRequest>();
  }

  if (request->matches_.empty()) {
    request->matches_.emplace_back(); /* if not filled, remove before return */
  }

  YLayoutBox *vbox = fac->createVBox(dialog);

  const string kUpdateRuleDialogTitle = "Update Firewall Rule";
  const string kInsertRuleDialogTitle = "Insert Firewall Rule";
  if (index == std::nullopt) {
    auto *title = fac->createLabel(vbox, kInsertRuleDialogTitle);
    title->autoWrap();
  } else {
    auto *title = fac->createLabel(vbox, kUpdateRuleDialogTitle);
    title->autoWrap();
  }

  WidgetManager collector;
  {
    /* Line 1: position, protocol and target */
    auto *hbox = fac->createHBox(vbox);

    /* for update dialog, user cannot modify rule_num */
    if (index.has_value()) {
      auto text = fmt::format("Rule to update: #{}", index.value());
      fac->createLabel(hbox, text);
    } else {
      auto rule_num = static_cast<int>(
          firewall_backend_->getFirewallChildren(firewall_context_).size());
      auto text = fmt::format("Rule #(1-{})", rule_num);
      auto *pos_input = fac->createIntField(hbox, text, 1, rule_num + 1, 1);
      collector.addWidget(pos_input, [pos_input, &request]() {
        auto *widget = dynamic_cast<YIntField *>(pos_input);
        request->index_ = widget->value();
        return HandleResult::SUCCESS;
      });
    }

    auto ptcs = protocols();
    YComboBox *proto_box = fac->createComboBox(hbox, "Protocol");
    YItemCollection items;
    for (const auto &ptc : ptcs) {
      items.push_back(new YItem(get<0>(ptc)));
    }
    proto_box->addItems(items);
    if (index.has_value()) {
      proto_box->setValue(request->proto_);
    }
    collector.addWidget(proto_box, [proto_box, &request]() {
      auto *widget = dynamic_cast<YComboBox *>(proto_box);
      auto *item = widget->selectedItem();

      auto value = item->label();
      request->proto_ = value;
      return HandleResult::SUCCESS;
    });

    auto iptables_targets = iptTargets();
    YComboBox *target_box = fac->createComboBox(hbox, "Target");
    YItemCollection target_items;
    for (const auto &target : iptables_targets) {
      target_items.push_back(new YItem(target));
    }
    target_box->addItems(target_items);
    if (index.has_value() &&
        std::any_of(iptables_targets.begin(), iptables_targets.end(),
                    [request](const string &target) {
                      return target == request->target_;
                    })) {
      target_box->setValue(request->target_);
    }
    collector.addWidget(target_box, [target_box, &request]() {
      auto *widget = dynamic_cast<YComboBox *>(target_box);
      auto *item = widget->selectedItem();

      auto value = item->label();
      request->target_ = value;
      return HandleResult::SUCCESS;
    });
  }

  /* Line 2/3: source/dest addr and mask */
  constexpr int kIPMaxLen = 15;
  static const string kFFMask = "255.255.255.255";
  auto ip_input = [&](const string &target, optional<string> &addr_target,
                      optional<string> &mask_target) {
    auto *frame = YUI::widgetFactory()->createCheckBoxFrame(
        vbox, target + " Address / Mask", false);
    frame->setAutoEnable(true); /* if frame is enabled, child set enabled */

    {
      auto *hbox = fac->createHBox(frame);
      auto *addr = fac->createInputField(hbox, target + " Address");
      addr->setInputMaxLength(kIPMaxLen);
      if (index.has_value() && addr_target.has_value()) {
        addr->setValue(addr_target.value());
      }

      auto *mask = fac->createInputField(hbox, target + " Mask");
      mask->setInputMaxLength(kIPMaxLen);
      if (index.has_value() && mask_target.has_value()) {
        mask->setValue(mask_target.value());
      } else {
        mask_target = kFFMask;
        mask->setValue(kFFMask);
      }

      collector.addWidget(
          frame, [hbox, addr, mask, &addr_target, &mask_target]() {
            if (hbox->isEnabled()) {
              auto *addr_widget = dynamic_cast<YInputField *>(addr);
              addr_target = addr_widget->value();

              auto *mask_widget = dynamic_cast<YInputField *>(mask);
              mask_target = mask_widget->value();
            }
            return HandleResult::SUCCESS;
          });
    }
  };

  ip_input("Source", request->src_ip_, request->src_mask_);
  ip_input("Dest", request->dst_ip_, request->dst_mask_);

  /* Line 4/5: iniface and outiface */
  {
    auto *frame =
        YUI::widgetFactory()->createCheckBoxFrame(vbox, "Iniface", false);
    frame->setAutoEnable(true);

    auto *iface = fac->createInputField(frame, "");
    if (index.has_value() && request->iniface_.has_value()) {
      iface->setValue(request->iniface_.value());
    }

    collector.addWidget(frame, [iface, &request]() {
      if (iface->isEnabled()) {
        auto *iface_widget = dynamic_cast<YInputField *>(iface);
        request->iniface_ = iface_widget->value();
      }
      return HandleResult::SUCCESS;
    });
  }
  {
    auto *frame =
        YUI::widgetFactory()->createCheckBoxFrame(vbox, "Outiface", false);
    frame->setAutoEnable(true);

    auto *iface = fac->createInputField(frame, "");
    if (index.has_value() && request->outiface_.has_value()) {
      iface->setValue(request->outiface_.value());
    }

    collector.addWidget(frame, [iface, &request]() {
      if (iface->isEnabled()) {
        auto *iface_widget = dynamic_cast<YInputField *>(iface);
        request->outiface_ = iface_widget->value();
      }
      return HandleResult::SUCCESS;
    });
  }

  {
    constexpr int kPortMax = 65535;
    constexpr int kPortMin = 1;

    auto port_input = [&](const string &target,
                          optional<tuple<string, string>> &port_target) {
      auto *frame = YUI::widgetFactory()->createCheckBoxFrame(
          vbox, target + " Port", false);
      frame->setAutoEnable(true);

      auto *hbox = fac->createHBox(frame);
      auto *from = fac->createIntField(hbox, target + " from", kPortMin,
                                       kPortMax, kPortMin);
      auto *to = fac->createIntField(hbox, target + " to", kPortMin, kPortMax,
                                     kPortMax);
      if (index.has_value() && port_target.has_value()) {
        auto [from_str, to_str] = port_target.value();
        from->setValue(std::stoi(from_str));
        to->setValue(std::stoi(to_str));
      }

      collector.addWidget(frame, [hbox, from, to, &port_target, &request]() {
        if (hbox->isEnabled()) {
          auto *to_widget = dynamic_cast<YIntField *>(to);
          auto *from_widget = dynamic_cast<YIntField *>(from);

          if (to_widget->value() < from_widget->value()) {
            return HandleResult::ERROR;
          }

          port_target = std::make_tuple("", "");
          get<0>(port_target.value()) = std::to_string(from_widget->value());
          get<1>(port_target.value()) = std::to_string(to_widget->value());
        }
        return HandleResult::SUCCESS;
      });
    };

    port_input("Source", request->matches_.front().src_port_range_);
    port_input("Dest", request->matches_.front().dst_port_range_);
  }

  auto *control_layout = fac->createHBox(vbox);
  auto *confirm = fac->createPushButton(control_layout, "&OK");
  fac->createHSpacing(control_layout, button_space);
  auto *cancel = fac->createPushButton(control_layout, "&Cancel");

  while (true) {
    auto *event = dialog->waitForEvent();
    if (event->widget() == confirm) {
      if (!collector.exec()) {
        static const string kInvalidInput = "Invalid input.";
        showDialog(dialog_meta::ERROR, kInvalidInput);
        continue;
      }

      // special case for port range
      if (!request->matches_.front().src_port_range_.has_value() &&
          !request->matches_.front().dst_port_range_.has_value()) {
        request->matches_.clear();
      }
      dialog->destroy();
      return request;
    }

    if (event->widget() == cancel ||
        event->eventType() == YEvent::CancelEvent) {
      break;
    }
  }

  dialog->destroy();
  return nullptr;
}

auto FirewallConfig::createChain() -> shared_ptr<ChainRequest> {
  static constexpr int button_space = 5;

  auto *fac = getFactory();
  YDialog *dialog = fac->createPopupDialog();

  auto request = std::make_shared<ChainRequest>();
  YLayoutBox *vbox = fac->createVBox(dialog);

  auto *title = fac->createLabel(vbox, "Create Firewall Chain");
  title->autoWrap();

  WidgetManager collector;
  auto *name = fac->createInputField(vbox, "Chain Name");
  collector.addWidget(name, [name, &request]() {
    auto *widget = dynamic_cast<YInputField *>(name);
    request->chain_name_ = widget->value();
    return HandleResult::SUCCESS;
  });

  auto *control_layout = fac->createHBox(vbox);
  auto *confirm = fac->createPushButton(control_layout, "&OK");
  fac->createHSpacing(control_layout, button_space);
  auto *cancel = fac->createPushButton(control_layout, "&Cancel");

  while (true) {
    auto *event = dialog->waitForEvent();
    if (event->widget() == confirm) {
      if (!collector.exec()) {
        static const string kInvalidInput = "Invalid input.";
        showDialog(dialog_meta::ERROR, kInvalidInput);
        break;
      }

      dialog->destroy();
      return request;
    }

    if (event->widget() == cancel ||
        event->eventType() == YEvent::CancelEvent) {
      break;
    }
  }

  return nullptr;
}
