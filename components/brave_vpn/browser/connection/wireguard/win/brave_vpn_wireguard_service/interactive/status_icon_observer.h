/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_INTERACTIVE_STATUS_ICON_OBSERVER_H_
#define BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_INTERACTIVE_STATUS_ICON_OBSERVER_H_

#include "build/build_config.h"

class StatusIconObserver {
 public:
  // Called when the user clicks on the system tray icon. Clicks that result
  // in the context menu being displayed will not be passed to this observer
  // (i.e. if there's a context menu set on this status icon, and the user
  // right clicks on the icon to display the context menu, OnStatusIconClicked()
  // will not be called).
  // Note: Chrome OS displays the context menu on left button clicks.
  // This will only be fired for this platform if no context menu is present.
  virtual void OnStatusIconClicked() = 0;

#if BUILDFLAG(IS_WIN)
  // Called when the user clicks on a balloon generated for a system tray icon.
  // TODO(dewittj): Implement on platforms other than Windows.  Currently this
  // event will never fire on non-Windows platforms.
  virtual void OnBalloonClicked() {}
#endif

 protected:
  virtual ~StatusIconObserver() {}
};

#endif  // BRAVE_COMPONENTS_BRAVE_VPN_BROWSER_CONNECTION_WIREGUARD_WIN_BRAVE_VPN_WIREGUARD_SERVICE_INTERACTIVE_STATUS_ICON_OBSERVER_H_
