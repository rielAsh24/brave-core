/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/status_icon.h"

#include <utility>

#include "base/observer_list.h"
#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/status_icon_observer.h"
#include "build/build_config.h"

StatusIcon::StatusIcon() {}

StatusIcon::~StatusIcon() {}

void StatusIcon::AddObserver(StatusIconObserver* observer) {
  observers_.AddObserver(observer);
}

void StatusIcon::RemoveObserver(StatusIconObserver* observer) {
  observers_.RemoveObserver(observer);
}

bool StatusIcon::HasObservers() const {
  return !observers_.empty();
}

void StatusIcon::DispatchClickEvent() {
  for (StatusIconObserver& observer : observers_) {
    observer.OnStatusIconClicked();
  }
}

void StatusIcon::ForceVisible() {}

void StatusIcon::SetContextMenu(std::unique_ptr<StatusIconMenuModel> menu) {
  // The UI may been showing a menu for the current model, don't destroy it
  // until we've notified the UI of the change.
  std::unique_ptr<StatusIconMenuModel> old_menu =
    std::move(context_menu_contents_);
  context_menu_contents_ = std::move(menu);
  UpdatePlatformContextMenu(context_menu_contents_.get());
}
