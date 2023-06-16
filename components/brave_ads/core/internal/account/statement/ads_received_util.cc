/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/account/statement/ads_received_util.h"

#include "base/ranges/algorithm.h"
#include "base/time/time.h"

namespace brave_ads {

base::flat_map<std::string, int32_t> GetAdTypesReceivedForDateRange(
    const TransactionList& transactions,
    base::Time from_time,
    base::Time to_time) {
  base::flat_map<std::string, int32_t> map;
  for (const auto& transaction : transactions) {
    if (transaction.confirmation_type == ConfirmationType::kViewed &&
        transaction.created_at >= from_time &&
        transaction.created_at <= to_time) {
      map[transaction.ad_type.ToString()] += 1;
    }
  }
  return map;
}

}  // namespace brave_ads
