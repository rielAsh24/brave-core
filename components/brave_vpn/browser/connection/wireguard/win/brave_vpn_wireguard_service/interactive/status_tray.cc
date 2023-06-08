/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/status_tray.h"

#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/status_icon.h"

StatusTray::~StatusTray() {}

StatusIcon* StatusTray::CreateStatusIcon(const gfx::ImageSkia& image,
                                         const std::u16string& tool_tip) {
  auto icon = CreatePlatformStatusIcon(type, image, tool_tip);
  if (!icon) {
    return nullptr;
  }

  status_icons_.push_back(std::move(icon));
  return status_icons_.back().get();
}

void StatusTray::RemoveStatusIcon(StatusIcon* icon) {
  for (auto iter = status_icons_.begin(); iter != status_icons_.end(); ++iter) {
    if (iter->get() == icon) {
      status_icons_.erase(iter);
      return;
    }
  }
  NOTREACHED();
}

StatusTray::StatusTray() {}
