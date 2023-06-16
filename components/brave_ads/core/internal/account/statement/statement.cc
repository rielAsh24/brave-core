/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/account/statement/statement.h"

#include <numeric>
#include <utility>

#include "base/functional/bind.h"
#include "base/functional/callback.h"
#include "base/time/time.h"
#include "brave/components/brave_ads/common/interfaces/brave_ads.mojom.h"
#include "brave/components/brave_ads/core/internal/account/statement/statement_util.h"
#include "brave/components/brave_ads/core/internal/account/transactions/transactions.h"
#include "brave/components/brave_ads/core/internal/common/logging_util.h"
#include "brave/components/brave_ads/core/internal/common/time/time_util.h"

namespace brave_ads {

void BuildStatement(BuildStatementCallback callback) {
  const base::Time from_time = GetTimeInDistantPast();
  const base::Time to_time = GetLocalTimeAtEndOfThisMonth();

  GetTransactionsForDateRange(
      from_time, to_time,
      base::BindOnce(
          [](BuildStatementCallback callback, const bool success,
             const TransactionList& transactions) {
            if (!success) {
              BLOG(0, "Failed to get transactions");
              return std::move(callback).Run(/*statement*/ nullptr);
            }

            mojom::StatementInfoPtr statement = mojom::StatementInfo::New();
            const auto [min_last_month, max_last_month] =
                GetEstimatedEarningsForLastMonth(transactions);
            statement->min_earnings_last_month = min_last_month;
            statement->max_earnings_last_month = max_last_month;
            const auto [min_this_month, max_this_month] =
                GetEstimatedEarningsForThisMonth(transactions);
            statement->min_earnings_this_month = min_this_month;
            statement->max_earnings_this_month = max_this_month;
            statement->next_payment_date = GetNextPaymentDate(transactions);
            statement->ad_types_received_this_month =
                GetAdTypesReceivedThisMonth(transactions);
            statement->ads_received_this_month = std::reduce(
                statement->ad_types_received_this_month.begin(),
                statement->ad_types_received_this_month.end(),
                static_cast<int32_t>(0),
                [](int32_t sum, auto& entry) { return sum + entry.second; });

            std::move(callback).Run(std::move(statement));
          },
          std::move(callback)));
}

}  // namespace brave_ads
