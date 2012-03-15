// Copyright (c) 2006-2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cef_paths.h"
#include "include/internal/cef_string.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/string_util.h"
#include "base/sys_info.h"
#include "chrome/common/chrome_constants.h"
#include "chrome/common/chrome_paths_internal.h"
#include "chrome/common/chrome_switches.h"


namespace {

// File name of the internal Flash plugin on different platforms.
const FilePath::CharType kInternalFlashPluginFileName[] =
#if defined(OS_MACOSX)
    FILE_PATH_LITERAL("Flash Player Plugin for Chrome.plugin");
#elif defined(OS_WIN)
    FILE_PATH_LITERAL("gcswf32.dll");
#else  // OS_LINUX, etc.
    FILE_PATH_LITERAL("libgcflashplayer.so");
#endif

}  // namespace

namespace cef {

// Gets the path for internal plugins.
bool GetInternalPluginsDirectory(FilePath* result) {
  // The rest of the world expects plugins in the module directory.

  bool res = PathService::Get(base::DIR_MODULE, result);
#ifdef OS_LINUX
  *result = result->Append("../lib");
#endif
  return res;
}

bool GetFlashPluginPath(FilePath* result, const char* dllName)
{
  FilePath cur;

  if (!GetInternalPluginsDirectory(&cur))
    return false;

  if (dllName)
	cur = cur.Append(CefStringUTF16(dllName));
  else
	cur = cur.Append(kInternalFlashPluginFileName);

  if (!file_util::PathExists(cur))
    return false;

  *result = cur;
  return true;
}




}  // namespace chrome
