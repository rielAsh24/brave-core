/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_ads/core/internal/account/user_data/locale_user_data.h"

#include "base/test/values_test_util.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_base.h"
#include "brave/components/brave_ads/core/internal/common/unittest/unittest_mock_util.h"
#include "brave/components/l10n/common/test/scoped_default_locale.h"

// npm run test -- brave_unit_tests --filter=BraveAds*

namespace brave_ads {

class BraveAdsLocaleUserDataTest : public UnitTestBase {};

TEST_F(BraveAdsLocaleUserDataTest,
       DoNotBuildLocaleUserDataForNonReleaseBuildChannel) {
  // Arrange
  MockBuildChannel(BuildChannelType::kNightly);

  // Act
  const base::Value::Dict user_data = BuildLocaleUserData();

  // Assert
  EXPECT_TRUE(user_data.empty());
}

TEST_F(BraveAdsLocaleUserDataTest, BuildLocaleUserDataForReleaseBuildChannel) {
  // Arrange
  MockBuildChannel(BuildChannelType::kRelease);

  // Act

  // Assert
  EXPECT_EQ(base::test::ParseJsonDict(R"({"countryCode":"US"})"),
            BuildLocaleUserData());
}

TEST_F(BraveAdsLocaleUserDataTest,
       DoNotBuildLocaleUserDataForCountryNotInAnonymitySet) {
  // Arrange
  const brave_l10n::test::ScopedDefaultLocale scoped_default_locale{"en_MC"};

  MockBuildChannel(BuildChannelType::kRelease);

  // Act
  const base::Value::Dict user_data = BuildLocaleUserData();

  // Assert
  EXPECT_TRUE(user_data.empty());
}

TEST_F(BraveAdsLocaleUserDataTest,
       BuildLocaleUserDataForCountryNotInAnonymitySetButShouldClassifyAsOther) {
  // Arrange
  const brave_l10n::test::ScopedDefaultLocale scoped_default_locale{"en_CX"};

  MockBuildChannel(BuildChannelType::kRelease);

  // Act

  // Assert
  EXPECT_EQ(base::test::ParseJsonDict(R"({"countryCode":"??"})"),
            BuildLocaleUserData());
}

}  // namespace brave_ads
