/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ACCOUNT_ISSUERS_PUBLIC_KEY_ALIAS_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ACCOUNT_ISSUERS_PUBLIC_KEY_ALIAS_H_

#include <string>

#include "base/containers/flat_map.h"

namespace ads {

using PublicKeyMap = base::flat_map<std::string, double>;

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_ACCOUNT_ISSUERS_PUBLIC_KEY_ALIAS_H_
