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
#include "ui/views/controls/menu/native_menu_win.h"
#include "ui/views/layout/layout_provider.h"

namespace gfx {
class Point;
}

class StatusTrayWin;
class StatusIconMenuModel;

class StatusIconWin {
 public:
  // Constructor which provides this icon's unique ID and messaging window.
  StatusIconWin(StatusTrayWin* tray, UINT id, HWND window, UINT message);

  StatusIconWin(const StatusIconWin&) = delete;
  StatusIconWin& operator=(const StatusIconWin&) = delete;

  ~StatusIconWin();

  // Handles a click event from the user - if |left_button_click| is true and
  // there is a registered observer, passes the click event to the observer,
  // otherwise displays the context menu if there is one.
  void HandleClickEvent(const gfx::Point& cursor_pos, bool left_button_click);

  // Re-creates the status tray icon now after the taskbar has been created.
  void ResetIcon();

  UINT icon_id() const { return icon_id_; }
  HWND window() const { return window_; }
  UINT message_id() const { return message_id_; }

  void SetImage(const gfx::ImageSkia& image);
  void SetToolTip(const std::u16string& tool_tip);
  void SetContextMenu(std::unique_ptr<StatusIconMenuModel> menu);
  void OnMenuCommand(int index, int event_flags);

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

  // Context menu, if any.
  std::unique_ptr<StatusIconMenuModel> menu_model_;

  // Context menu associated with this icon (if any).
  std::unique_ptr<views::NativeMenuWin> native_menu_;
};

#endif  // BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_INTERACTIVE_STATUS_ICON_WIN_H_
