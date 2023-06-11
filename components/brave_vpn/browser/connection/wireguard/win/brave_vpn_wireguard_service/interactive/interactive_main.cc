/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/interactive_main.h"

#include <wrl/client.h>

#include "base/logging.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"
#include "ui/base/models/image_model.h"
#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/resources/resource.h"
#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/status_icon_win.h"
#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/status_tray_win.h"
#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/brave_vpn_tray_command_ids.h"
#include "ui/base/models/image_model.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/icon_util.h"
#include "ui/gfx/image/image_family.h"

namespace brave_vpn {
namespace {
std::unique_ptr<gfx::ImageFamily> GetAppIconImageFamily(int icon_id) {
  // Get the icon from the current module.
  HMODULE module = GetModuleHandle(nullptr);
  DCHECK(module);
  return IconUtil::CreateImageFamilyFromIconResource(module, icon_id);
}

gfx::ImageSkia GetIconFromResources(int icon_id, gfx::Size size) {
  std::unique_ptr<gfx::ImageFamily> family = GetAppIconImageFamily(icon_id);
  DCHECK(family);
  if (!family) {
    return gfx::ImageSkia();
  }

  return family->CreateExact(size).AsImageSkia();
}

}  // namespace

InteractiveMain* InteractiveMain::GetInstance() {
  static base::NoDestructor<InteractiveMain> instance;
  return instance.get();
}

InteractiveMain::InteractiveMain() = default;

InteractiveMain::~InteractiveMain() = default;

void InteractiveMain::SetupStatusIcon() {
  ui::ResourceBundle::InitSharedInstanceWithLocale(
      "en", /*delegate=*/nullptr,
      ui::ResourceBundle::DO_NOT_LOAD_COMMON_RESOURCES);

  status_tray_ = std::make_unique<StatusTrayWin>();

  status_icon_ =
      status_tray_->CreateStatusIcon(GetIconFromResources(IDR_BRAVE_VPN_TRAY_DARK, {64, 64}), u"BraveVpn");

  std::unique_ptr<StatusIconMenuModel> menu(new StatusIconMenuModel(this));
  //ui::ImageModel::FromImageSkia(GetIconFromResources(IDR_BRAVE_VPN_TOGGLER_ON, {64, 64})));
  menu->AddItem(IDC_BRAVE_VPN_TRAY_CONNECT_VPN, u"Connect Brave VPN");
  menu->AddItem(IDC_BRAVE_VPN_TRAY_DISCONNECT_VPN, u"Disconnect Brave VPN");
  menu->AddItem(IDC_BRAVE_VPN_TRAY_HIDE_TRAY_ICON, u"Hide tray icon");
  status_icon_->SetContextMenu(std::move(menu));
}
HRESULT InteractiveMain::Run() {
  base::SingleThreadTaskExecutor service_task_executor(
      base::MessagePumpType::UI);
  base::RunLoop loop;
  quit_ = loop.QuitClosure();
  SetupStatusIcon();
  loop.Run();

  return S_OK;
}

void InteractiveMain::SignalExit() {
  std::move(quit_).Run();
}

void InteractiveMain::ExecuteCommand(int command_id, int event_flags) {
  LOG(ERROR) << __func__ << ":" << command_id;
  switch(command_id) {
    case IDC_BRAVE_VPN_TRAY_HIDE_TRAY_ICON:
      SignalExit();
    break;
  }
}

}  // namespace brave_vpn
