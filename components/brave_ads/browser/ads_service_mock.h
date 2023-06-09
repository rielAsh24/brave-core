/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_ADS_BROWSER_ADS_SERVICE_MOCK_H_
#define BRAVE_COMPONENTS_BRAVE_ADS_BROWSER_ADS_SERVICE_MOCK_H_

#include <string>
#include <vector>

#include "brave/components/brave_ads/browser/ads_service.h"
#include "brave/components/brave_ads/common/interfaces/brave_ads.mojom-shared.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace brave_ads {

class AdsServiceMock : public AdsService {
 public:
  AdsServiceMock();

  AdsServiceMock(const AdsServiceMock&) = delete;
  AdsServiceMock& operator=(const AdsServiceMock&) = delete;

  AdsServiceMock(AdsServiceMock&&) noexcept = delete;
  AdsServiceMock& operator=(AdsServiceMock&&) noexcept = delete;

  ~AdsServiceMock() override;

  MOCK_CONST_METHOD0(IsEnabled, bool());

  MOCK_CONST_METHOD0(GetMaximumNotificationAdsPerHour, int64_t());
  MOCK_METHOD1(SetMaximumNotificationAdsPerHour, void(int64_t));

  MOCK_CONST_METHOD0(ShouldAllowSubdivisionTargeting, bool());
  MOCK_CONST_METHOD0(GetSubdivisionTargetingCode, std::string());
  MOCK_METHOD1(SetSubdivisionTargetingCode, void(const std::string&));
  MOCK_CONST_METHOD0(GetAutoDetectedSubdivisionTargetingCode, std::string());
  MOCK_METHOD1(SetAutoDetectedSubdivisionTargetingCode,
               void(const std::string&));

  MOCK_CONST_METHOD0(NeedsBrowserUpgradeToServeAds, bool());

  MOCK_METHOD2(ShowScheduledCaptcha,
               void(const std::string&, const std::string&));
  MOCK_METHOD0(SnoozeScheduledCaptcha, void());

  MOCK_METHOD1(OnNotificationAdShown, void(const std::string&));
  MOCK_METHOD2(OnNotificationAdClosed, void(const std::string&, bool));
  MOCK_METHOD1(OnNotificationAdClicked, void(const std::string&));

  MOCK_METHOD1(GetDiagnostics, void(GetDiagnosticsCallback));

  MOCK_METHOD2(OnDidUpdateResourceComponent,
               void(const std::string&, const std::string&));

  MOCK_METHOD1(GetStatementOfAccounts, void(GetStatementOfAccountsCallback));

  MOCK_METHOD2(MaybeServeInlineContentAd,
               void(const std::string&,
                    MaybeServeInlineContentAdAsDictCallback));
  MOCK_METHOD4(TriggerInlineContentAdEvent,
               void(const std::string&,
                    const std::string&,
                    mojom::InlineContentAdEventType,
                    TriggerAdEventCallback));

  MOCK_METHOD0(GetPrefetchedNewTabPageAdForDisplay,
               absl::optional<NewTabPageAdInfo>());
  MOCK_METHOD0(PrefetchNewTabPageAd, void());
  MOCK_METHOD4(TriggerNewTabPageAdEvent,
               void(const std::string&,
                    const std::string&,
                    mojom::NewTabPageAdEventType,
                    TriggerAdEventCallback));
  MOCK_METHOD2(OnFailedToPrefetchNewTabPageAd,
               void(const std::string&, const std::string&));

  MOCK_METHOD4(TriggerPromotedContentAdEvent,
               void(const std::string&,
                    const std::string&,
                    mojom::PromotedContentAdEventType,
                    TriggerAdEventCallback));

  MOCK_METHOD3(TriggerSearchResultAdEvent,
               void(mojom::SearchResultAdInfoPtr,
                    const mojom::SearchResultAdEventType,
                    TriggerAdEventCallback));

  MOCK_METHOD2(PurgeOrphanedAdEventsForType,
               void(mojom::AdType, PurgeOrphanedAdEventsForTypeCallback));

  MOCK_METHOD3(GetHistory, void(base::Time, base::Time, GetHistoryCallback));

  MOCK_METHOD2(ToggleLikeAd, void(base::Value::Dict, ToggleLikeAdCallback));
  MOCK_METHOD2(ToggleDislikeAd,
               void(base::Value::Dict, ToggleDislikeAdCallback));
  MOCK_METHOD3(ToggleLikeCategory,
               void(const std::string&,
                    mojom::UserReactionType,
                    ToggleLikeCategoryCallback));
  MOCK_METHOD3(ToggleDislikeCategory,
               void(const std::string&,
                    mojom::UserReactionType,
                    ToggleDislikeCategoryCallback));
  MOCK_METHOD2(ToggleSaveAd, void(base::Value::Dict, ToggleSaveAdCallback));
  MOCK_METHOD2(ToggleMarkAdAsInappropriate,
               void(base::Value::Dict, ToggleMarkAdAsInappropriateCallback));

  MOCK_METHOD3(NotifyTabTextContentDidChange,
               void(int32_t tab_id,
                    const std::vector<GURL>& redirect_chain,
                    const std::string& text));
  MOCK_METHOD3(NotifyTabHtmlContentDidChange,
               void(int32_t tab_id,
                    const std::vector<GURL>& redirect_chain,
                    const std::string& html));
  MOCK_METHOD1(NotifyTabDidStartPlayingMedia, void(int32_t tab_id));
  MOCK_METHOD1(NotifyTabDidStopPlayingMedia, void(int32_t tab_id));
  MOCK_METHOD3(NotifyTabDidChange,
               void(int32_t tab_id,
                    const std::vector<GURL>& redirect_chain,
                    bool is_visible));
  MOCK_METHOD1(NotifyDidCloseTab, void(int32_t tab_id));
  MOCK_METHOD1(NotifyUserGestureEventTriggered, void(int32_t));
  MOCK_METHOD0(NotifyBrowserDidBecomeActive, void());
  MOCK_METHOD0(NotifyBrowserDidResignActive, void());

  MOCK_METHOD0(NotifyDidSolveAdaptiveCaptcha, void());
};

}  // namespace brave_ads

#endif  // BRAVE_COMPONENTS_BRAVE_ADS_BROWSER_ADS_SERVICE_MOCK_H_
