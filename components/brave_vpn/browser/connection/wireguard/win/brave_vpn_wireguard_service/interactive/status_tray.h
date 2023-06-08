/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_INTERACTIVE_STATUS_ICON_WIN_H_
#define BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_INTERACTIVE_STATUS_ICON_WIN_H_

#include <memory>
#include <string>
#include <vector>

namespace gfx {
class ImageSkia;
}

class StatusIcon;

// Provides a cross-platform interface to the system's status tray, and exposes
// APIs to add/remove icons to the tray and attach context menus.
class StatusTray {
 public:
  // Static factory method that is implemented separately for each platform to
  // produce the appropriate platform-specific instance. Returns NULL if this
  // platform does not support status icons.
  static std::unique_ptr<StatusTray> Create();

  StatusTray(const StatusTray&) = delete;
  StatusTray& operator=(const StatusTray&) = delete;

  virtual ~StatusTray();

  // Creates a new StatusIcon. The StatusTray retains ownership of the
  // StatusIcon. Returns NULL if the StatusIcon could not be created.
  StatusIcon* CreateStatusIcon(const gfx::ImageSkia& image,
                               const std::u16string& tool_tip);

  // Removes |icon| from this status tray.
  void RemoveStatusIcon(StatusIcon* icon);

 protected:
  using StatusIcons = std::vector<std::unique_ptr<StatusIcon>>;

  StatusTray();

  // Factory method for creating a status icon for this platform.
  virtual std::unique_ptr<StatusIcon> CreatePlatformStatusIcon(
      const gfx::ImageSkia& image,
      const std::u16string& tool_tip) = 0;

  // Returns the list of active status icons so subclasses can operate on them.
  const StatusIcons& status_icons() const { return status_icons_; }

 private:
  // List containing all active StatusIcons. The icons are owned by this
  // StatusTray.
  StatusIcons status_icons_;
};

#endif  // BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_INTERACTIVE_STATUS_ICON_WIN_H_
