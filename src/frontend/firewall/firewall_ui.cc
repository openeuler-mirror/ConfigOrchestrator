#include "frontend/firewall/firewall_ui.h"
#include "backend/config_manager.h"
#include "backend/firewall/firewall_context.h"
#include "controlpanel.h"
#include "fmt/core.h"
#include "frontend/ui_base.h"
#include "tools/iptools.h"
#include "tools/uitools.h"

#include <algorithm>
#include <arpa/inet.h>
#include <cstring>
#include <optional>
#include <string>
#include <tuple>
#include <vector>
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

#include <memory>
#include <sstream>
#include <utility>

using std::stringstream;

const string FirewallConfig::kNonSuWarnText =
    "Please run the control panel as root to configure firewall.";
const string FirewallConfig::kAddRuleButtonText = "&Add Firewall Rule";
const string FirewallConfig::kAddChainButtonText = "&Add Firewall Chain";
const string FirewallConfig::kDelRuleButtonText = "Delete";
const string FirewallConfig::kDelChainButtonText = "&Delete Firewall Chain";

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

auto FirewallConfig::fresh(YDialog *main_dialog, DisplayLayout layout) // NOLINT
    -> bool {
  auto res = true;
  iptable_children = firewall_backend_->getFirewallChildren(firewall_context_);

  auto *fac = getFactory();
  auto *main_layout = layout.feature_layout_;

  // TODO(yiyan): remove children from tragets
  for (auto it = main_layout->childrenBegin(); it != main_layout->childrenEnd();
       it++) {
    for (auto widget_it = widgets_targets_.begin();
         widget_it != widgets_targets_.end(); widget_it++) {
      if (get<0>(*widget_it) == *it) {
        widgets_targets_.erase(widget_it);
        break;
      }
    }
  }
  main_layout->deleteChildren();
  switch (firewall_context_->level_) {
  case FirewallLevel::OVERALL:
  case FirewallLevel::TABLE: {
    for (const auto &child : iptable_children) {
      auto *button = fac->createPushButton(main_layout, child);
      widgets_targets_.emplace_back(button, [this, button, child]() {
        auto context =
            firewall_backend_->createContext(firewall_context_, child);

        auto subpage = std::make_shared<FirewallConfig>(
            child, shared_from_this(), context);

        subpage->display();
        subpage->handleEvent();
        return true;
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

      widgets_targets_.emplace_back(detail_button, [this, index]() {
        auto title = fmt::format("Rule Detail: #{}", index);
        showDialog(title,
                   firewall_backend_->getRuleDetails(firewall_context_, index));
        return true;
      });

      widgets_targets_.emplace_back(del_button, [this, iptable_child, index,
                                                 main_dialog, layout]() {
        auto res = firewall_backend_->removeRule(firewall_context_, index);

        if (!res) {
          auto msg = fmt::format(
              "Failed to remove chain: #{}\nChain brief: {}\nError: {}\n",
              index, iptable_child, firewall_context_->getLastError());
          showDialog(dialog_meta::ERROR, msg);
          return true;
        }

        ConfigManager::instance().registerApplyFunc(firewall_backend_->apply());
        fresh(main_dialog, layout);
        return true;
      });

      widgets_targets_.emplace_back(update_button, [this, iptable_child, index,
                                                    main_dialog, layout]() {
        auto request = createUpdateRule(index);
        if (request == nullptr) {
          return true; /* cancel */
        }

        auto res =
            firewall_backend_->updateRule(firewall_context_, request, index);
        if (!res) {
          auto msg = fmt::format(
              "Failed to update chain: #{}\nChain brief: {}\nError: {}\n",
              index, iptable_child, firewall_context_->getLastError());

          showDialog(dialog_meta::ERROR, msg);
          return true;
        }

        ConfigManager::instance().registerApplyFunc(firewall_backend_->apply());
        fresh(main_dialog, layout);
        return true;
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
    auto *add_chain_button_ =
        fac->createPushButton(control_layout, kAddChainButtonText);

    widgets_targets_.emplace_back(add_chain_button_, [this, main_dialog,
                                                      layout]() {
      auto requset = createChain();
      if (requset != nullptr) {
        auto res = firewall_backend_->insertChain(firewall_context_, requset);
        stringstream ss;

        if (!res) {
          ss << "Failed to add chain.";
          showDialog(dialog_meta::ERROR, ss.str());
        } else {
          stringstream ss;
          ss << "Chain added: " << requset->chain_name_;
          showDialog(dialog_meta::INFO, ss.str());
          ConfigManager::instance().registerApplyFunc(
              firewall_backend_->apply());

          fresh(main_dialog, layout);
        }
      }
      return true;
    });
    break;
  }
  case FirewallLevel::CHAIN: {
    auto *add_rule_button_ =
        fac->createPushButton(control_layout, kAddRuleButtonText);
    widgets_targets_.emplace_back(add_rule_button_, [this, main_dialog,
                                                     layout]() {
      auto request = createUpdateRule(std::nullopt);
      if (request != nullptr) {
        auto res = firewall_backend_->insertRule(firewall_context_, request);
        if (!res) {
          auto msg = fmt::format("Failed to add rule, Error: {}\n",
                                 firewall_context_->getLastError());
          showDialog(dialog_meta::ERROR, msg);
          return true;
        }

        ConfigManager::instance().registerApplyFunc(firewall_backend_->apply());
        fresh(main_dialog, layout);
      }

      return true;
    });

    auto *del_chain_button_ =
        fac->createPushButton(control_layout, kDelChainButtonText);
    widgets_targets_.emplace_back(del_chain_button_, [this, main_dialog,
                                                      layout]() {
      auto res = firewall_backend_->removeChain(firewall_context_);
      stringstream ss;

      if (!res) {
        ss << "Failed to remove chain: " << firewall_context_->serialize();
        showDialog(dialog_meta::ERROR, ss.str());
      } else {
        ss << "Chain removed: " << firewall_context_->serialize();
        showDialog(dialog_meta::INFO, ss.str());

        ConfigManager::instance().registerApplyFunc(firewall_backend_->apply());
        fresh(main_dialog, layout);
      }

      return true;
    });
    break;
  }
  }

  /* main layout */
  fresh(main_dialog, layout);

  return DisplayResult::SUCCESS;
}

auto FirewallConfig::userHandleEvent(YEvent *event) -> HandleResult {
  auto res = HandleResult::CONT;
  for (const auto &[widget, func] : widgets_targets_) {
    if (event->widget() == widget) {
      func();

      res = HandleResult::SUCCESS;
      break;
    }
  }

  return res;
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

auto FirewallConfig::createUpdateRule(optional<int> index)
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

  vector<widget_func_t> widget_targets;
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
      widget_targets.emplace_back(pos_input, [pos_input, &request]() {
        auto *widget = dynamic_cast<YIntField *>(pos_input);
        request->index_ = widget->value();
        return true;
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
    widget_targets.emplace_back(proto_box, [proto_box, &request]() {
      auto *widget = dynamic_cast<YComboBox *>(proto_box);
      auto *item = widget->selectedItem();

      auto value = item->label();
      request->proto_ = value;
      return true;
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
    widget_targets.emplace_back(target_box, [target_box, &request]() {
      auto *widget = dynamic_cast<YComboBox *>(target_box);
      auto *item = widget->selectedItem();

      auto value = item->label();
      request->target_ = value;
      return true;
    });
  }

  /* Line 2/3: source/dest addr and mask */
  constexpr int kIPMaxLen = 15;
  static const string kFFMask = "255.255.255.255";
  auto ip_input = [&](const string &target, optional<string> &addr_target,
                      optional<string> &mask_target) {
    auto *frame = YUI::widgetFactory()->createCheckBoxFrame(
        vbox, target + " Address / Mask", false);

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

      widget_targets.emplace_back(
          frame, [frame, addr, mask, &addr_target, &mask_target]() {
            auto *widget = dynamic_cast<YCheckBoxFrame *>(frame);
            if (widget->isEnabled()) {
              auto *addr_widget = dynamic_cast<YInputField *>(addr);
              addr_target = addr_widget->value();

              auto *mask_widget = dynamic_cast<YInputField *>(mask);
              mask_target = mask_widget->value();
            }
            return true;
          });
    }
  };

  ip_input("Source", request->src_ip_, request->src_mask_);
  ip_input("Dest", request->dst_ip_, request->dst_mask_);

  /* Line 4/5: iniface and outiface */
  {
    auto *frame =
        YUI::widgetFactory()->createCheckBoxFrame(vbox, "Iniface", false);
    auto *iface = fac->createInputField(frame, "");
    if (index.has_value() && request->iniface_.has_value()) {
      iface->setValue(request->iniface_.value());
    }

    widget_targets.emplace_back(frame, [frame, iface, &request]() {
      auto *widget = dynamic_cast<YCheckBoxFrame *>(frame);
      if (widget->isEnabled()) {
        auto *iface_widget = dynamic_cast<YInputField *>(iface);
        request->iniface_ = iface_widget->value();
      }
      return true;
    });
  }
  {
    auto *frame =
        YUI::widgetFactory()->createCheckBoxFrame(vbox, "Outiface", false);
    auto *iface = fac->createInputField(frame, "");
    if (index.has_value() && request->outiface_.has_value()) {
      iface->setValue(request->outiface_.value());
    }

    widget_targets.emplace_back(frame, [frame, iface, &request]() {
      auto *widget = dynamic_cast<YCheckBoxFrame *>(frame);
      if (widget->isEnabled()) {
        auto *iface_widget = dynamic_cast<YInputField *>(iface);
        request->outiface_ = iface_widget->value();
      }
      return true;
    });
  }

  {
    constexpr int kPortMax = 65535;
    constexpr int kPortMin = 1;

    auto port_input = [&](const string &target,
                          optional<tuple<string, string>> &port_target) {
      auto *frame = YUI::widgetFactory()->createCheckBoxFrame(
          vbox, target + " Port", false);

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

      widget_targets.emplace_back(frame, [frame, from, to, &port_target,
                                          &request]() {
        auto *widget = dynamic_cast<YCheckBoxFrame *>(frame);
        if (widget->isEnabled()) {
          auto *to_widget = dynamic_cast<YIntField *>(to);
          auto *from_widget = dynamic_cast<YIntField *>(from);

          if (to_widget->value() < from_widget->value()) {
            return false;
          }

          port_target = std::make_tuple("", "");
          get<0>(port_target.value()) = std::to_string(from_widget->value());
          get<1>(port_target.value()) = std::to_string(to_widget->value());
        }
        return true;
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
      bool flag = true;
      for (const auto &[_, func] : widget_targets) {
        flag &= func();
        if (!flag) {
          static const string kInvalidInput = "Invalid input.";
          showDialog(dialog_meta::ERROR, kInvalidInput);
          break;
        }
      }

      if (flag) {
        // special case for port range
        if (!request->matches_.front().src_port_range_.has_value() &&
            !request->matches_.front().dst_port_range_.has_value()) {
          request->matches_.clear();
        }
        dialog->destroy();
        return request;
      }
    }

    if (event->widget() == cancel ||
        event->eventType() == YEvent::CancelEvent) {
      dialog->destroy();
      return nullptr;
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

  vector<widget_func_t> widget_targets;
  auto *name = fac->createInputField(vbox, "Chain Name");
  widget_targets.emplace_back(name, [name, &request]() {
    auto *widget = dynamic_cast<YInputField *>(name);
    request->chain_name_ = widget->value();
    return true;
  });

  auto *control_layout = fac->createHBox(vbox);
  auto *confirm = fac->createPushButton(control_layout, "&OK");
  fac->createHSpacing(control_layout, button_space);
  auto *cancel = fac->createPushButton(control_layout, "&Cancel");

  while (true) {
    auto *event = dialog->waitForEvent();
    if (event->widget() == confirm) {
      bool flag = true;
      for (const auto &[_, func] : widget_targets) {
        flag &= func();
        if (!flag) {
          static const string kInvalidInput = "Invalid input.";
          showDialog(dialog_meta::ERROR, kInvalidInput);
          break;
        }
      }

      if (flag) {
        dialog->destroy();
        return request;
      }
    }

    if (event->widget() == cancel ||
        event->eventType() == YEvent::CancelEvent) {
      dialog->destroy();
      return nullptr;
    }
  }

  return nullptr;
}
