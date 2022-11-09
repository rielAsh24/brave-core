/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_SPEEDREADER_SPEEDREADER_SERVICE_H_
#define BRAVE_COMPONENTS_SPEEDREADER_SPEEDREADER_SERVICE_H_

#include <string>

#include "base/memory/raw_ptr.h"
#include "brave/components/speedreader/common/speedreader_panel.mojom.h"
#include "brave/components/speedreader/speedreader_util.h"
#include "components/keyed_service/core/keyed_service.h"

class PrefRegistrySimple;
class PrefService;

namespace speedreader {
using mojom::ContentStyle;
using mojom::FontFamily;
using mojom::FontSize;
using mojom::Theme;

class SpeedreaderService : public KeyedService {
 public:
  explicit SpeedreaderService(PrefService* prefs);
  ~SpeedreaderService() override;

  static void RegisterProfilePrefs(PrefRegistrySimple* registry);

  void ToggleSpeedreader();
  void DisableSpeedreaderForTest();
  bool IsEnabled();
  bool ShouldPromptUserToEnable() const;
  void IncrementPromptCount();

  void SetTheme(Theme theme_name);
  Theme GetTheme() const;
  std::string GetThemeName() const;

  void SetFontSize(FontSize size);
  FontSize GetFontSize() const;
  std::string GetFontSizeName() const;

  void SetFontFamily(FontFamily font);
  FontFamily GetFontFamily() const;
  std::string GetFontFamilyName() const;

  void SetContentStyle(ContentStyle style);
  ContentStyle GetContentStyle() const;
  std::string GetContentStyleName() const;

  SpeedreaderService(const SpeedreaderService&) = delete;
  SpeedreaderService& operator=(const SpeedreaderService&) = delete;

 private:
  raw_ptr<PrefService> prefs_ = nullptr;
};

}  // namespace speedreader

#endif  // BRAVE_COMPONENTS_SPEEDREADER_SPEEDREADER_SERVICE_H_
