/* Copyright (c) 2023 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "src/content/browser/aggregation_service/features.cc"

#include "base/feature_override.h"

namespace content {

OVERRIDE_FEATURE_DEFAULT_STATES({{
    {aggregation_service::kAttributionReportingNullAggregatableReports,
     base::FEATURE_DISABLED_BY_DEFAULT},
    {aggregation_service::kAggregationServiceMultipleCloudProviders,
     base::FEATURE_DISABLED_BY_DEFAULT},
}});

}  // namespace content
