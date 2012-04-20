// Copyright (c) 2010 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/compiler_specific.h"

#include "third_party/WebKit/Source/WebCore/config.h"

#include "browser_webkit_glue.h"

#undef LOG
#include "base/file_util.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "grit/webkit_resources.h"
#include "ui/base/resource/data_pack.h"
#include "ui/base/resource/resource_bundle.h"
#include "webkit/glue/webkit_glue.h"

namespace webkit_glue {
  
// Data pack resource. This is a pointer to the mmapped resources file.
static ui::DataPack* g_resource_data_pack = NULL;


const char* PATH_ENV = "CEF_DATA_PATH";

// Helper method for getting the path to the CEF resources directory.
FilePath GetResourcesFilePath() {
  return FilePath(getenv(PATH_ENV));
}
  
void InitializeDataPak() {
  g_resource_data_pack = new ui::DataPack;
  FilePath data_path;
  data_path = data_path.Append(GetResourcesFilePath());
  data_path = data_path.Append("cef_data.pak");

  if (!g_resource_data_pack->Load(data_path)) {
    LOG(FATAL) << "failed to load cef_data.pak";
  }
  ResourceBundle::InitSharedInstanceForTest(data_path); // resources
}
  
string16 GetLocalizedString(int message_id) {
  base::StringPiece res;
  if (!g_resource_data_pack->GetStringPiece(message_id, &res)) {
    LOG(FATAL) << "failed to load webkit string with id " << message_id;
  }
  
  return string16(reinterpret_cast<const char16*>(res.data()),
                  res.length() / 2);
}
  
  
base::StringPiece NetResourceProvider(int key) {
  base::StringPiece res;
  g_resource_data_pack->GetStringPiece(key, &res);
  return res;
}  

base::StringPiece GfxResourceProvider(int key) {
  base::StringPiece res;
  g_resource_data_pack->GetStringPiece(key, &res);
  return res;
}  

base::StringPiece GetDataResource(int resource_id) {


  switch (resource_id) {
	case IDR_BROKENIMAGE:
	case IDR_TEXTAREA_RESIZER:
    case IDR_SEARCH_CANCEL:
    case IDR_SEARCH_CANCEL_PRESSED:
    case IDR_SEARCH_MAGNIFIER:
    case IDR_SEARCH_MAGNIFIER_RESULTS:
    case IDR_MEDIA_PAUSE_BUTTON:
    case IDR_MEDIA_PLAY_BUTTON:
    case IDR_MEDIA_PLAY_BUTTON_DISABLED:
    case IDR_MEDIA_SOUND_FULL_BUTTON:
    case IDR_MEDIA_SOUND_NONE_BUTTON:
    case IDR_MEDIA_SOUND_DISABLED:
    case IDR_MEDIA_SLIDER_THUMB:
    case IDR_MEDIA_VOLUME_SLIDER_THUMB:
    case IDR_INPUT_SPEECH:
    case IDR_INPUT_SPEECH_RECORDING:
    case IDR_INPUT_SPEECH_WAITING:
      return NetResourceProvider(resource_id);
      
    default:
      break;
  }
  
  return base::StringPiece();
}

void DidLoadPlugin(const std::string& filename) {
}

void DidUnloadPlugin(const std::string& filename) {
}
  
}  // namespace webkit_glue
