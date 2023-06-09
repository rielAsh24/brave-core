/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_INTERACTIVE_STATUS_ICON_WIN_H_
#define BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_INTERACTIVE_STATUS_ICON_WIN_H_

#include <windows.h>  // windows.h must be included before shellapi.h

#include <shellapi.h>
#include <memory>

#include "base/memory/raw_ptr.h"
#include "base/win/scoped_gdi_object.h"
#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/status_icon.h"
#include "ui/views/layout/layout_provider.h"

namespace gfx {
class Point;
}

namespace views {
class MenuRunner;
}

class StatusTrayWin;

class StatusIconWin : public StatusIcon {
 public:
  // Constructor which provides this icon's unique ID and messaging window.
  StatusIconWin(StatusTrayWin* tray, UINT id, HWND window, UINT message);

  StatusIconWin(const StatusIconWin&) = delete;
  StatusIconWin& operator=(const StatusIconWin&) = delete;

  ~StatusIconWin() override;

  // Handles a click event from the user - if |left_button_click| is true and
  // there is a registered observer, passes the click event to the observer,
  // otherwise displays the context menu if there is one.
  void HandleClickEvent(const gfx::Point& cursor_pos, bool left_button_click);

  // Re-creates the status tray icon now after the taskbar has been created.
  void ResetIcon();

  UINT icon_id() const { return icon_id_; }
  HWND window() const { return window_; }
  UINT message_id() const { return message_id_; }

  // Overridden from StatusIcon:
  void SetImage(const gfx::ImageSkia& image) override;
  void SetToolTip(const std::u16string& tool_tip) override;
  void ForceVisible() override;

 protected:
  // Overridden from StatusIcon:
  void UpdatePlatformContextMenu(StatusIconMenuModel* menu) override;

 private:
  void InitIconData(NOTIFYICONDATA* icon_data);

  // The tray that owns us.  Weak.
  raw_ptr<StatusTrayWin> tray_;

  // The unique ID corresponding to this icon.
  UINT icon_id_;

  // Window used for processing messages from this icon.
  HWND window_;

  // The message identifier used for status icon messages.
  UINT message_id_;
  views::LayoutProvider provider;
  // The currently-displayed icon for the window.
  base::win::ScopedHICON icon_;

  // Not owned.
  raw_ptr<ui::MenuModel> menu_model_ = nullptr;

  // Context menu associated with this icon (if any).
  std::unique_ptr<views::MenuRunner> menu_runner_;
};

#endif  // BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_INTERACTIVE_STATUS_ICON_WIN_H_
