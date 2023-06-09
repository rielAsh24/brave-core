/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_INTERACTIVE_MAIN_H_
#define BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_INTERACTIVE_MAIN_H_

#include "base/functional/callback.h"
#include "base/no_destructor.h"
#include "base/win/windows_types.h"

class StatusIcon;
class StatusTray;

namespace brave_vpn {

class InteractiveMain {
 public:
  static InteractiveMain* GetInstance();

  InteractiveMain(const InteractiveMain&) = delete;
  InteractiveMain& operator=(const InteractiveMain&) = delete;

  void SetupStatusIcon();

  HRESULT Run();
  void SignalExit();

 private:
  friend class base::NoDestructor<InteractiveMain>;

  InteractiveMain();
  ~InteractiveMain();

  std::unique_ptr<StatusTray> status_tray_;
  // Reference to our status icon (if any) - owned by the StatusTray.
  raw_ptr<StatusIcon, DanglingUntriaged> status_icon_ = nullptr;

  base::OnceClosure quit_;
};

}  // namespace brave_vpn

#endif  // BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_INTERACTIVE_MAIN_H_
