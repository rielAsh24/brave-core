/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/interactive_main.h"

#include <wrl/client.h>

#include "base/logging.h"
#include "base/run_loop.h"
#include "base/task/single_thread_task_executor.h"
#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/interactive_resource.h"
#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/status_icon.h"
#include "brave/components/brave_vpn/browser/connection/wireguard/win/brave_vpn_wireguard_service/interactive/status_tray.h"
#include "ui/base/models/image_model.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/geometry/size.h"
#include "ui/gfx/icon_util.h"
#include "ui/gfx/image/image_family.h"

namespace brave_vpn {
namespace {
std::unique_ptr<gfx::ImageFamily> GetAppIconImageFamily() {
  const int icon_id = IDR_MAINFRAME;
  // Get the icon from the current module.
  HMODULE module = GetModuleHandle(nullptr);
  DCHECK(module);
  return IconUtil::CreateImageFamilyFromIconResource(module, icon_id);
}

gfx::ImageSkia GetStatusTrayIcon() {
  gfx::Size size(128, 128);
  std::unique_ptr<gfx::ImageFamily> family = GetAppIconImageFamily();
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

  status_tray_ = StatusTray::Create();

  status_icon_ =
      status_tray_->CreateStatusIcon(GetStatusTrayIcon(), u"BraveVpn");

  std::unique_ptr<StatusIconMenuModel> menu(new StatusIconMenuModel(this));
  menu->AddItem(1000, u"Turn On Brave Vpn");
  menu->AddItem(1001, u"Exit");
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
}

}  // namespace brave_vpn
