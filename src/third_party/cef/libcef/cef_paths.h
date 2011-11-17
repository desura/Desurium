// Copyright (c) 2006-2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CEF_COMMON_CHROME_PATHS_H__
#define CEF_COMMON_CHROME_PATHS_H__
#pragma once

#include "build/build_config.h"

class FilePath;

namespace cef {

  bool GetInternalPluginsDirectory(FilePath* result);
  bool GetFlashPluginPath(FilePath* result, const char* dllName);

}  // namespace cef

#endif  // CEF_COMMON_CHROME_PATHS_H__
