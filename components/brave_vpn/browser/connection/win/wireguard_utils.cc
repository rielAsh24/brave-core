/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_vpn/browser/connection/win/wireguard_utils.h"

#include <objbase.h>
#include <stdint.h>
#include <wrl/client.h>
#include <utility>

#include "base/base64.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/task/single_thread_task_runner.h"
#include "base/task/thread_pool.h"
#include "base/win/com_init_util.h"
#include "base/win/scoped_bstr.h"
#include "brave/components/brave_vpn/browser/connection/win/brave_vpn_wireguard_service/brave_wireguard_manager_idl.h"
#include "brave/components/brave_vpn/browser/connection/win/brave_vpn_wireguard_service/service_constants.h"

namespace brave_vpn {

namespace {
// Template for wireguard config generation.
constexpr char kWireguardConfigTemplate[] = R"(
  [Interface]
  PrivateKey = {client_private_key}
  Address = {mapped_ipv4_address}
  DNS = {dns_servers}
  [Peer]
  PublicKey = {server_public_key}
  AllowedIPs = 0.0.0.0/0, ::/0
  Endpoint = {vpn_server_hostname}:51821
)";

}  // namespace

namespace internal {

absl::optional<std::string> CreateWireguardConfig(
    const std::string& client_private_key,
    const std::string& server_public_key,
    const std::string& vpn_server_hostname,
    const std::string& mapped_ipv4_address,
    const std::string& dns_servers) {
  if (client_private_key.empty() || server_public_key.empty() ||
      vpn_server_hostname.empty() || mapped_ipv4_address.empty() ||
      dns_servers.empty()) {
    return absl::nullopt;
  }
  std::string config = kWireguardConfigTemplate;
  base::ReplaceSubstringsAfterOffset(&config, 0, "{client_private_key}",
                                     client_private_key);
  base::ReplaceSubstringsAfterOffset(&config, 0, "{server_public_key}",
                                     server_public_key);
  base::ReplaceSubstringsAfterOffset(&config, 0, "{vpn_server_hostname}",
                                     vpn_server_hostname);
  base::ReplaceSubstringsAfterOffset(&config, 0, "{mapped_ipv4_address}",
                                     mapped_ipv4_address);
  base::ReplaceSubstringsAfterOffset(&config, 0, "{dns_servers}", dns_servers);
  return config;
}

bool StartVpnWGServiceImpl(const std::string& config) {
  base::win::AssertComInitialized();
  HRESULT hr = S_OK;
  Microsoft::WRL::ComPtr<IBraveWireguardManager> service;
  CoCreateInstance(
      brave_vpn::GetBraveWireguardServiceClsid(), nullptr, CLSCTX_LOCAL_SERVER,
      brave_vpn::GetBraveWireguardServiceIid(), IID_PPV_ARGS_Helper(&service));
  if (FAILED(hr)) {
    VLOG(1) << "Unable to create IBraveWireguardManager instance";
    return false;
  }

  hr = CoSetProxyBlanket(
      service.Get(), RPC_C_AUTHN_DEFAULT, RPC_C_AUTHZ_DEFAULT,
      COLE_DEFAULT_PRINCIPAL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
      RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_DYNAMIC_CLOAKING);
  if (FAILED(hr)) {
    VLOG(1) << "Unable to call EnableVpn interface";
    return false;
  }
  std::string encoded_config;
  base::Base64Encode(config, &encoded_config);
  DWORD error_code = 0;
  if (FAILED(service->EnableVpn(base::UTF8ToWide(encoded_config).c_str(),
                                &error_code))) {
    VLOG(1) << "Unable to call EnableVpn interface";
    return false;
  }
  return error_code == 0;
}

void StartVpnWGService(const std::string& config, BooleanCallback callback) {
  base::ThreadPool::CreateCOMSTATaskRunner(
      {base::MayBlock(), base::WithBaseSyncPrimitives(),
       base::TaskPriority::BEST_EFFORT,
       base::TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN},
      base::SingleThreadTaskRunnerThreadMode::DEDICATED)
      ->PostTaskAndReplyWithResult(
          FROM_HERE, base::BindOnce(&StartVpnWGServiceImpl, config),
          std::move(callback));
}

bool StopVpnWGServiceImpl() {
  base::win::AssertComInitialized();
  HRESULT hr = S_OK;
  Microsoft::WRL::ComPtr<IBraveWireguardManager> service;
  CoCreateInstance(
      brave_vpn::GetBraveWireguardServiceClsid(), nullptr, CLSCTX_LOCAL_SERVER,
      brave_vpn::GetBraveWireguardServiceIid(), IID_PPV_ARGS_Helper(&service));
  if (FAILED(hr)) {
    VLOG(1) << "Unable to create IBraveVpnService instance";
    return false;
  }

  hr = CoSetProxyBlanket(
      service.Get(), RPC_C_AUTHN_DEFAULT, RPC_C_AUTHZ_DEFAULT,
      COLE_DEFAULT_PRINCIPAL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
      RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_DYNAMIC_CLOAKING);
  if (FAILED(hr)) {
    VLOG(1) << "Unable to call EnableVpn interface";
    return false;
  }
  DWORD error_code = 0;
  if (FAILED(service->DisableVpn(&error_code))) {
    VLOG(1) << "Unable to call EnableVpn interface";
    return false;
  }
  return error_code == 0;
}

void StopVpnWGService(BooleanCallback callback) {
  base::ThreadPool::CreateCOMSTATaskRunner(
      {base::MayBlock(), base::WithBaseSyncPrimitives(),
       base::TaskPriority::BEST_EFFORT,
       base::TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN},
      base::SingleThreadTaskRunnerThreadMode::DEDICATED)
      ->PostTaskAndReplyWithResult(FROM_HERE,
                                   base::BindOnce(&StopVpnWGServiceImpl),
                                   std::move(callback));
}

WireguardKeyPair WireGuardGenerateKeypairImpl() {
  base::win::AssertComInitialized();
  HRESULT hr = S_OK;
  Microsoft::WRL::ComPtr<IBraveWireguardManager> service;
  CoCreateInstance(
      brave_vpn::GetBraveWireguardServiceClsid(), nullptr, CLSCTX_LOCAL_SERVER,
      brave_vpn::GetBraveWireguardServiceIid(), IID_PPV_ARGS_Helper(&service));
  if (FAILED(hr)) {
    VLOG(1) << "Unable to create IBraveWireguardManager instance";
    return absl::nullopt;
  }

  hr = CoSetProxyBlanket(
      service.Get(), RPC_C_AUTHN_DEFAULT, RPC_C_AUTHZ_DEFAULT,
      COLE_DEFAULT_PRINCIPAL, RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
      RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_DYNAMIC_CLOAKING);
  if (FAILED(hr)) {
    VLOG(1) << "Unable to call EnableVpn interface";
    return absl::nullopt;
  }

  DWORD error_code = 0;
  base::win::ScopedBstr public_key_raw;
  base::win::ScopedBstr private_key_raw;
  if (FAILED(service->GenerateKeypair(
          public_key_raw.Receive(), private_key_raw.Receive(), &error_code))) {
    VLOG(1) << "Unable to call EnableVpn interface";
    return absl::nullopt;
  }

  std::wstring public_key_wide;
  public_key_wide.assign(
      reinterpret_cast<std::wstring::value_type*>(public_key_raw.Get()),
      public_key_raw.Length());
  std::wstring private_key_wide;
  private_key_wide.assign(
      reinterpret_cast<std::wstring::value_type*>(private_key_raw.Get()),
      private_key_raw.Length());
  std::string public_key = base::WideToUTF8(public_key_wide);
  std::string private_key = base::WideToUTF8(private_key_wide);
  return std::make_tuple(public_key, private_key);
}

void WireGuardGenerateKeypair(WireGuardGenerateKeypairCallback callback) {
  base::ThreadPool::CreateCOMSTATaskRunner(
      {base::MayBlock(), base::WithBaseSyncPrimitives(),
       base::TaskPriority::BEST_EFFORT,
       base::TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN},
      base::SingleThreadTaskRunnerThreadMode::DEDICATED)
      ->PostTaskAndReplyWithResult(
          FROM_HERE, base::BindOnce(&WireGuardGenerateKeypairImpl),
          std::move(callback));
}

}  // namespace internal

}  // namespace brave_vpn
