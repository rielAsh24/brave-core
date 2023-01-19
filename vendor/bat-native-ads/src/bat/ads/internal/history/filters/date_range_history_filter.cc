/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/history/filters/date_range_history_filter.h"

#include <algorithm>

namespace ads {

DateRangeHistoryFilter::DateRangeHistoryFilter(const base::Time from_time,
                                               const base::Time to_time)
    : from_time_(from_time), to_time_(to_time) {}

HistoryItemList DateRangeHistoryFilter::Apply(
    const HistoryItemList& history) const {
  HistoryItemList filtered_history = history;

  const auto iter =
      std::remove_if(filtered_history.begin(), filtered_history.end(),
                     [=](const HistoryItemInfo& history_item) {
                       return history_item.created_at < from_time_ ||
                              history_item.created_at > to_time_;
                     });

  filtered_history.erase(iter, filtered_history.cend());

  return filtered_history;
}

}  // namespace ads
