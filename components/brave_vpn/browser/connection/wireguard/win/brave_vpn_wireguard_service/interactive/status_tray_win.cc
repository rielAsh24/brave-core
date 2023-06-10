/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/status_tray_win.h"

#include <commctrl.h>
#include <wrl/client.h>

#include <utility>

#include "base/files/file_path.h"
#include "base/functional/bind.h"
#include "base/location.h"
#include "base/sequence_checker.h"
#include "base/task/single_thread_task_runner.h"
#include "base/threading/thread.h"
#include "base/win/windows_types.h"
#include "base/win/wrapped_window_proc.h"
#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive//status_tray_state_changer_win.h"
#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/status_icon_win.h"
#include "ui/display/win/screen_win.h"
#include "ui/gfx/geometry/point.h"
#include "ui/gfx/geometry/point_conversions.h"
#include "ui/gfx/win/hwnd_util.h"

static const UINT kStatusIconMessage = WM_APP + 1;

namespace {
// |kBaseIconId| is 2 to avoid conflicts with plugins that hard-code id 1.
const UINT kBaseIconId = 2;
const base::FilePath::CharType kStatusTrayWindowClass[] =
    FILE_PATH_LITERAL("BraveVpn_StatusTrayWindow");

const base::FilePath::CharType kBraveVpnTaskbarMessageName[] =
    FILE_PATH_LITERAL("BraveVpnTaskbarCreated");

gfx::Point GetCursorScreenPoint() {
  POINT pt;
  ::GetCursorPos(&pt);
  return gfx::ToFlooredPoint(
      display::win::ScreenWin::ScreenToDIPPoint(gfx::PointF(gfx::Point(pt))));
}

}  // namespace

// Default implementation for StatusTrayStateChanger that communicates to
// Exporer.exe via COM.  It spawns a background thread with a fresh COM
// apartment and requests that the visibility be increased unless the user
// has explicitly set the icon to be hidden.
class StatusTrayStateChangerProxyImpl : public StatusTrayStateChangerProxy {
 public:
  StatusTrayStateChangerProxyImpl()
      : pending_requests_(0), worker_thread_("StatusIconCOMWorkerThread") {
    worker_thread_.init_com_with_mta(false);
  }

  StatusTrayStateChangerProxyImpl(const StatusTrayStateChangerProxyImpl&) =
      delete;
  StatusTrayStateChangerProxyImpl& operator=(
      const StatusTrayStateChangerProxyImpl&) = delete;

  ~StatusTrayStateChangerProxyImpl() override {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  }

  void EnqueueChange(UINT icon_id, HWND window) override {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    if (pending_requests_ == 0) {
      worker_thread_.Start();
    }

    ++pending_requests_;
    worker_thread_.task_runner()->PostTaskAndReply(
        FROM_HERE,
        base::BindOnce(
            &StatusTrayStateChangerProxyImpl::EnqueueChangeOnWorkerThread,
            icon_id, window),
        base::BindOnce(&StatusTrayStateChangerProxyImpl::ChangeDone,
                       weak_factory_.GetWeakPtr()));
  }

 private:
  // Must be called only on |worker_thread_|, to ensure the correct COM
  // apartment.
  static void EnqueueChangeOnWorkerThread(UINT icon_id, HWND window) {
    Microsoft::WRL::ComPtr<StatusTrayStateChangerWin>
        status_tray_state_changer =
            Microsoft::WRL::Make<StatusTrayStateChangerWin>(icon_id, window);
    status_tray_state_changer->EnsureTrayIconVisible();
  }

  // Called on UI thread.
  void ChangeDone() {
    DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
    DCHECK_GT(pending_requests_, 0);

    if (--pending_requests_ == 0) {
      worker_thread_.Stop();
    }
  }

 private:
  int pending_requests_;
  base::Thread worker_thread_;

  SEQUENCE_CHECKER(sequence_checker_);

  base::WeakPtrFactory<StatusTrayStateChangerProxyImpl> weak_factory_{this};
};

StatusTrayWin::StatusTrayWin()
    : next_icon_id_(1), atom_(0), instance_(NULL), window_(NULL) {
  // Register our window class
  WNDCLASSEX window_class;
  base::win::InitializeWindowClass(
      kStatusTrayWindowClass,
      &base::win::WrappedWindowProc<StatusTrayWin::WndProcStatic>, 0, 0, 0,
      NULL, NULL, NULL, NULL, NULL, &window_class);
  instance_ = window_class.hInstance;
  atom_ = RegisterClassEx(&window_class);
  CHECK(atom_);

  // If the taskbar is re-created after we start up, we have to rebuild all of
  // our icons.
  taskbar_created_message_ = RegisterWindowMessage(kBraveVpnTaskbarMessageName);

  // Create an offscreen window for handling messages for the status icons. We
  // create a hidden WS_POPUP window instead of an HWND_MESSAGE window, because
  // only top-level windows such as popups can receive broadcast messages like
  // "BraveVpnTaskbarCreated".
  window_ = CreateWindow(MAKEINTATOM(atom_), 0, WS_POPUP, 0, 0, 0, 0, 0, 0,
                         instance_, 0);
  gfx::CheckWindowCreated(window_, ::GetLastError());
  gfx::SetWindowUserData(window_, this);
}

StatusTrayWin::~StatusTrayWin() {
  if (window_) {
    DestroyWindow(window_);
  }

  if (atom_) {
    UnregisterClass(MAKEINTATOM(atom_), instance_);
  }
}

void StatusTrayWin::UpdateIconVisibilityInBackground(
    StatusIconWin* status_icon) {
  if (!state_changer_proxy_.get()) {
    state_changer_proxy_ = std::make_unique<StatusTrayStateChangerProxyImpl>();
  }

  state_changer_proxy_->EnqueueChange(status_icon->icon_id(),
                                      status_icon->window());
}

LRESULT CALLBACK StatusTrayWin::WndProcStatic(HWND hwnd,
                                              UINT message,
                                              WPARAM wparam,
                                              LPARAM lparam) {
  StatusTrayWin* msg_wnd =
      reinterpret_cast<StatusTrayWin*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  if (msg_wnd) {
    return msg_wnd->WndProc(hwnd, message, wparam, lparam);
  } else {
    return ::DefWindowProc(hwnd, message, wparam, lparam);
  }
}

LRESULT CALLBACK StatusTrayWin::WndProc(HWND hwnd,
                                        UINT message,
                                        WPARAM wparam,
                                        LPARAM lparam) {
  int command_id = LOWORD(wparam);
  int event_flags = HIWORD(wparam);
  if (message == WM_COMMAND) {
    LOG(ERROR) << "WM_COMMAND:" << message << ", command_id:" << command_id << " - " << event_flags << " - wparam:" << wparam << ", lparam:" << lparam;
  }
  if (message == WM_MENUCOMMAND) {    
    LOG(ERROR) << "WM_MENUCOMMAND:" << command_id << " - " << event_flags << " - wparam:" << wparam
    << " -> lparam lo:" << LOWORD(lparam) << ", lparam hi:" << HIWORD(lparam);
  }
  
  if (message == taskbar_created_message_) {
    // We need to reset all of our icons because the taskbar went away.
    for (StatusIcons::const_iterator i(status_icons().begin());
         i != status_icons().end(); ++i) {
      StatusIconWin* win_icon = static_cast<StatusIconWin*>(i->get());
      win_icon->ResetIcon();
    }
    return TRUE;
  } else if (message == kStatusIconMessage) {
    StatusIconWin* win_icon = nullptr;

    // Find the selected status icon.
    for (StatusIcons::const_iterator i(status_icons().begin());
         i != status_icons().end(); ++i) {
      StatusIconWin* current_win_icon = static_cast<StatusIconWin*>(i->get());
      if (current_win_icon->icon_id() == wparam) {
        win_icon = current_win_icon;
        break;
      }
    }

    // It is possible for this procedure to be called with an obsolete icon
    // id.  In that case we should just return early before handling any
    // actions.
    if (!win_icon) {
      return TRUE;
    }
    switch (lparam) {
      case WM_LBUTTONDOWN:
      case WM_RBUTTONDOWN:
      case WM_CONTEXTMENU:
        // Walk our icons, find which one was clicked on, and invoke its
        // HandleClickEvent() method.
        gfx::Point cursor_pos(GetCursorScreenPoint());
        win_icon->HandleClickEvent(cursor_pos, lparam == WM_LBUTTONDOWN);
        return TRUE;
    }
  } else if (message == WM_ENDSESSION) {
    // If Chrome is in background-only mode, this is the only notification
    // it gets that Windows is exiting. Make sure we shutdown in an orderly
    // fashion.
    // chrome::SessionEnding();
  }
  return ::DefWindowProc(hwnd, message, wparam, lparam);
}

std::unique_ptr<StatusIcon> StatusTrayWin::CreatePlatformStatusIcon(
    const gfx::ImageSkia& image,
    const std::u16string& tool_tip) {
  UINT next_icon_id = kBaseIconId;
  auto icon = std::make_unique<StatusIconWin>(this, next_icon_id, window_,
                                              kStatusIconMessage);

  icon->SetImage(image);
  icon->SetToolTip(tool_tip);
  return std::move(icon);
}

void StatusTrayWin::SetStatusTrayStateChangerProxyForTest(
    std::unique_ptr<StatusTrayStateChangerProxy> proxy) {
  state_changer_proxy_ = std::move(proxy);
}

std::unique_ptr<StatusTray> StatusTray::Create() {
  return std::make_unique<StatusTrayWin>();
}
