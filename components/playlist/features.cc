/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/playlist/features.h"

#include "base/dcheck_is_on.h"
#include "base/feature_list.h"

namespace playlist::features {

#ifndef NDEBUG
const base::Feature kPlaylist{"Playlist", base::FEATURE_DISABLED_BY_DEFAULT};
#else
const base::Feature kPlaylist{"Playlist", base::FEATURE_ENABLED_BY_DEFAULT};
#endif // NDEBUG

}  // namespace playlist::features
