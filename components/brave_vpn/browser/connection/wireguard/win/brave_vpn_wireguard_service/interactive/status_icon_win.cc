/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/status_icon_win.h"

#include <string.h>

#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/status_tray_win.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/rect.h"
#include "ui/gfx/icon_util.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/views/controls/menu/menu_insertion_delegate_win.h"
#include "ui/views/controls/menu/menu_runner.h"

namespace {

class BraveMenuInsertionDelegateWin : public views::MenuInsertionDelegateWin {
 public:
  BraveMenuInsertionDelegateWin() {}

  BraveMenuInsertionDelegateWin(const BraveMenuInsertionDelegateWin&) = delete;
  BraveMenuInsertionDelegateWin& operator=(
      const BraveMenuInsertionDelegateWin&) = delete;

  ~BraveMenuInsertionDelegateWin() override {}

  // Overridden from views::MenuInsertionDelegateWin:
  size_t GetInsertionIndex(HMENU native_menu) override {
    native_menu_ = native_menu;
    return static_cast<size_t>(std::max(1, GetMenuItemCount(native_menu)) - 1);
  }

  HMENU GetNativeMenu() { return native_menu_; }

 private:
  HMENU native_menu_ = nullptr;
};

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// StatusIconWin, public:

StatusIconWin::StatusIconWin(StatusTrayWin* tray,
                             UINT id,
                             HWND window,
                             UINT message)
    : tray_(tray), icon_id_(id), window_(window), message_id_(message) {
  NOTIFYICONDATA icon_data;
  InitIconData(&icon_data);
  icon_data.uFlags = NIF_MESSAGE;
  icon_data.uCallbackMessage = message_id_;
  BOOL result = Shell_NotifyIcon(NIM_ADD, &icon_data);
  // This can happen if the explorer process isn't running when we try to
  // create the icon for some reason (for example, at startup).
  if (!result) {
    LOG(WARNING) << "Unable to create status tray icon.";
  }
}

StatusIconWin::~StatusIconWin() {
  // Remove our icon.
  NOTIFYICONDATA icon_data;
  InitIconData(&icon_data);
  Shell_NotifyIcon(NIM_DELETE, &icon_data);
}

void StatusIconWin::HandleClickEvent(const gfx::Point& cursor_pos,
                                     bool left_mouse_click) {
  // Pass to the observer if appropriate.
  if (left_mouse_click && HasObservers()) {
    DispatchClickEvent();
    return;
  }

  if (!menu_model_) {
    return;
  }

  // Set our window as the foreground window, so the context menu closes when
  // we click away from it.
  if (!SetForegroundWindow(window_)) {
    return;
  }

  system_menu_ = std::make_unique<views::NativeMenuWin>(menu_model_, nullptr);
  BraveMenuInsertionDelegateWin menu_delegate;
  system_menu_->Rebuild(&menu_delegate);

  TrackPopupMenu(menu_delegate.GetNativeMenu(), TPM_BOTTOMALIGN, cursor_pos.x(),
                 cursor_pos.y(), 0, window_, NULL);
}

void StatusIconWin::ResetIcon() {
  NOTIFYICONDATA icon_data;
  InitIconData(&icon_data);
  // Delete any previously existing icon.
  Shell_NotifyIcon(NIM_DELETE, &icon_data);
  InitIconData(&icon_data);
  icon_data.uFlags = NIF_MESSAGE;
  icon_data.uCallbackMessage = message_id_;
  icon_data.hIcon = icon_.get();
  // If we have an image, then set the NIF_ICON flag, which tells
  // Shell_NotifyIcon() to set the image for the status icon it creates.
  if (icon_data.hIcon) {
    icon_data.uFlags |= NIF_ICON;
  }
  // Re-add our icon.
  BOOL result = Shell_NotifyIcon(NIM_ADD, &icon_data);
  if (!result) {
    LOG(WARNING) << "Unable to re-create status tray icon.";
  }
}

void StatusIconWin::SetImage(const gfx::ImageSkia& image) {
  // Create the icon.
  NOTIFYICONDATA icon_data;
  InitIconData(&icon_data);
  icon_data.uFlags = NIF_ICON;
  icon_ = IconUtil::CreateHICONFromSkBitmap(*image.bitmap());
  icon_data.hIcon = icon_.get();
  BOOL result = Shell_NotifyIcon(NIM_MODIFY, &icon_data);
  if (!result) {
    LOG(WARNING) << "Error setting status tray icon image";
  }
}

void StatusIconWin::SetToolTip(const std::u16string& tool_tip) {
  // Create the icon.
  NOTIFYICONDATA icon_data;
  InitIconData(&icon_data);
  icon_data.uFlags = NIF_TIP;
  wcscpy_s(icon_data.szTip, base::as_wcstr(tool_tip));
  BOOL result = Shell_NotifyIcon(NIM_MODIFY, &icon_data);
  if (!result) {
    LOG(WARNING) << "Unable to set tooltip for status tray icon";
  }
}

void StatusIconWin::ForceVisible() {
  tray_->UpdateIconVisibilityInBackground(this);
}

////////////////////////////////////////////////////////////////////////////////
// StatusIconWin, private:

void StatusIconWin::UpdatePlatformContextMenu(StatusIconMenuModel* menu) {
  // |menu_model_| is about to be destroyed. Destroy the menu (which closes it)
  // so that it doesn't attempt to continue using |menu_model_|.
  DCHECK(menu);
  menu_model_ = menu;
}

void StatusIconWin::InitIconData(NOTIFYICONDATA* icon_data) {
  memset(icon_data, 0, sizeof(NOTIFYICONDATA));
  icon_data->cbSize = sizeof(NOTIFYICONDATA);

  icon_data->hWnd = window_;
  icon_data->uID = icon_id_;
}
