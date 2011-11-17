// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _WEBVIEW_HOST_H
#define _WEBVIEW_HOST_H

#include "base/basictypes.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/rect.h"
#include "webwidget_host.h"

#if defined(TOOLKIT_USES_GTK)
#include "webkit/plugins/npapi/gtk_plugin_container_manager.h"
#endif

struct WebPreferences;
class BrowserWebViewDelegate;

namespace WebKit {
class WebDevToolsAgentClient;
class WebView;
}

// This class is a simple NativeView-based host for a WebView
class WebViewHost : public WebWidgetHost {
 public:
  // The new instance is deleted once the associated NativeView is destroyed.
  // The newly created window should be resized after it is created, using the
  // MoveWindow (or equivalent) function.
#ifdef OS_WIN
  static WebViewHost* Create(gfx::NativeView parent_view,
                                 const gfx::Rect&,
                                 BrowserWebViewDelegate* delegate,
                                 PaintDelegate* paint_delegate,
								 WebKit::WebDevToolsAgentClient* dev_tools_client,
                                 const WebPreferences& prefs);
#else                                 
  static WebViewHost* Create(gfx::NativeView parent_view,
	                             BrowserWebViewDelegate* delegate,
	                             WebKit::WebDevToolsAgentClient* devtools_client,
	                             const WebPreferences& prefs);
#endif
  WebKit::WebView* webview() const;

#if defined(TOOLKIT_USES_GTK)
  ~WebViewHost();

  // Create a new plugin parent container for a given plugin XID.
  void CreatePluginContainer(gfx::PluginWindowHandle id);

  // Destroy the plugin parent container when a plugin has been destroyed.
  void DestroyPluginContainer(gfx::PluginWindowHandle id);

  webkit::npapi::GtkPluginContainerManager* plugin_container_manager() {
    return &plugin_container_manager_;
  }
#elif defined(OS_MACOSX)
  void SetIsActive(bool active);
#endif

#ifdef OS_WIN
 ~WebViewHost();
#endif

 protected:
#ifdef OS_WIN
  WebViewHost(BrowserWebViewDelegate* delegate);
  virtual bool WndProc(UINT message, WPARAM wparam, LPARAM lparam);
  void MouseEvent(UINT message, WPARAM wparam, LPARAM lparam);
#endif

#if defined(TOOLKIT_USES_GTK)
  // Helper class that creates and moves plugin containers.
  webkit::npapi::GtkPluginContainerManager plugin_container_manager_;
#endif

#ifdef OS_WIN
  BrowserWebViewDelegate* delegate_;
 #endif
};

#endif  // _WEBVIEW_HOST_H
