// Copyright (c) 2011 The Chromium Embedded Framework Authors.
// Portions copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "browser_webview_delegate.h"
#include "browser_impl.h"

#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#include "base/message_loop.h"
#include "base/string_util.h"
#include "net/base/net_errors.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebContextMenuData.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebCursorInfo.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebPopupMenu.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebDragData.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebFrame.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebImage.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebPoint.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebRect.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebView.h"
#include "ui/gfx/native_widget_types.h"
#include "ui/gfx/gtk_util.h"
#include "ui/gfx/point.h"
#include "webkit/glue/webdropdata.h"
#include "webkit/glue/webpreferences.h"
#include "webkit/glue/webkit_glue.h"
#include "webkit/glue/window_open_disposition.h"
#include "webkit/plugins/npapi/plugin_list.h"
#include "webkit/plugins/npapi/webplugin.h"
#include "webkit/plugins/npapi/webplugin_delegate_impl.h"

using webkit::npapi::WebPluginDelegateImpl;
using WebKit::WebContextMenuData;
using WebKit::WebCursorInfo;
using WebKit::WebDragData;
using WebKit::WebDragOperationsMask;
using WebKit::WebExternalPopupMenu;
using WebKit::WebExternalPopupMenuClient;
using WebKit::WebFrame;
using WebKit::WebImage;
using WebKit::WebNavigationPolicy;
using WebKit::WebPoint;
using WebKit::WebPopupMenuInfo;
using WebKit::WebRect;
using WebKit::WebWidget;
using WebKit::WebView;

#define AddMenuItem(unused_0, menu, unused_1, id, label, enabled, unused_2) MenuItemCreate(menu, label, id, enabled, this)
#define AddMenuSeparator(menu, id) MenuItemCreateSeperator(menu)

namespace {

enum SelectionClipboardType {
  TEXT_HTML,
  PLAIN_TEXT,
};

GdkAtom GetTextHtmlAtom() {
  GdkAtom kTextHtmlGdkAtom = gdk_atom_intern_static_string("text/html");
  return kTextHtmlGdkAtom;
}

void SelectionClipboardGetContents(GtkClipboard* clipboard,
    GtkSelectionData* selection_data, guint info, gpointer data) {
  // Ignore formats that we don't know about.
  if (info != TEXT_HTML && info != PLAIN_TEXT)
    return;

  WebView* webview = static_cast<WebView*>(data);
  WebFrame* frame = webview->focusedFrame();
  if (!frame)
    frame = webview->mainFrame();
  DCHECK(frame);

  std::string selection;
  if (TEXT_HTML == info) {
    selection = frame->selectionAsMarkup().utf8();
  } else {
    selection = frame->selectionAsText().utf8();
  }
  if (TEXT_HTML == info) {
    gtk_selection_data_set(selection_data,
                           GetTextHtmlAtom(),
                           8 /* bits per data unit, ie, char */,
                           reinterpret_cast<const guchar*>(selection.data()),
                           selection.length());
  } else {
    gtk_selection_data_set_text(selection_data, selection.data(),
        selection.length());
  }
}

}  // namespace

// WebViewClient --------------------------------------------------------------

WebExternalPopupMenu* BrowserWebViewDelegate::createExternalPopupMenu(
    const WebPopupMenuInfo& info,
    WebExternalPopupMenuClient* client)  {
  NOTREACHED();
  return NULL;
}

static gboolean MenuItemHandle(GtkWidget* menu_item, gpointer data)
{
  if (!data)
    return FALSE;

  BrowserWebViewDelegate* webViewDelegate = (BrowserWebViewDelegate*)data;
  int id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(menu_item), "menu_id"));

  webViewDelegate->handleContextMenu(id);

  return FALSE;
}

static GtkWidget* MenuItemCreate(GtkWidget* parent_menu, const char* name, int id, bool is_enabled, BrowserWebViewDelegate* webViewDelegate)
{
  GtkWidget* menu_item = gtk_menu_item_new_with_label(name);

  g_object_set_data(G_OBJECT(menu_item), "menu_id", (gpointer)id);
  g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(MenuItemHandle), (gpointer)webViewDelegate);
  gtk_menu_shell_append(GTK_MENU_SHELL(parent_menu), menu_item);
  gtk_widget_set_sensitive(menu_item, is_enabled);

  gtk_widget_show(menu_item);

  return menu_item;
}

static GtkWidget* MenuItemCreateSeperator(GtkWidget* parent_menu)
{
  GtkWidget* menu_item = gtk_menu_item_new();

  gtk_menu_shell_append(GTK_MENU_SHELL(parent_menu), menu_item);
  gtk_widget_show(menu_item);

  return menu_item;
}

void BrowserWebViewDelegate::showContextMenu(WebKit::WebFrame* frame, const WebKit::WebContextMenuData& data) {
  GtkWidget* menu = NULL;
  GdkPoint screen_pt = { data.mousePosition.x, data.mousePosition.y };

  std::list<std::wstring> label_list;

  // Enable recursive tasks on the message loop so we can get updates while
  // the context menu is being displayed.
  bool old_state = MessageLoop::current()->NestableTasksAllowed();
  MessageLoop::current()->SetNestableTasksAllowed(true);

  int edit_flags = data.editFlags;
  if(browser_->UIT_CanGoBack())
    edit_flags |= MENU_CAN_GO_BACK;
  if(browser_->UIT_CanGoForward())
    edit_flags |= MENU_CAN_GO_FORWARD;

  int type_flags = MENUTYPE_NONE;
  if(!data.pageURL.isEmpty())
    type_flags |= MENUTYPE_PAGE;
  if(!data.frameURL.isEmpty())
    type_flags |= MENUTYPE_FRAME;
  if(!data.linkURL.isEmpty())
    type_flags |= MENUTYPE_LINK;
  if(data.mediaType == WebContextMenuData::MediaTypeImage)
    type_flags |= MENUTYPE_IMAGE;
  if(!data.selectedText.isEmpty())
    type_flags |= MENUTYPE_SELECTION;
  if(data.isEditable)
    type_flags |= MENUTYPE_EDITABLE;
  if(data.isSpellCheckingEnabled && !data.misspelledWord.isEmpty())
    type_flags |= MENUTYPE_MISSPELLED_WORD;
  if(data.mediaType == WebContextMenuData::MediaTypeVideo)
    type_flags |= MENUTYPE_VIDEO;
  if(data.mediaType == WebContextMenuData::MediaTypeAudio)
    type_flags |= MENUTYPE_AUDIO;
  
  CefRefPtr<CefClient> client = browser_->GetClient();
  CefRefPtr<CefMenuHandler> handler;
  if (client.get())
    handler = client->GetMenuHandler();

  if (handler.get()) {
    // Gather menu information
    cef_handler_menuinfo_t menuInfo;
    memset(&menuInfo, 0, sizeof(menuInfo));

    CefString linkStr(std::string(data.linkURL.spec()));
    CefString imageStr(std::string(data.srcURL.spec()));
    CefString pageStr(std::string(data.pageURL.spec()));
    CefString frameStr(std::string(data.frameURL.spec()));
    CefString selectedTextStr(string16(data.selectedText));
    CefString misspelledWordStr(string16(data.misspelledWord));
    CefString securityInfoStr(std::string(data.securityInfo));
    
    menuInfo.typeFlags = type_flags;
    menuInfo.x = screen_pt.x;
    menuInfo.y = screen_pt.y;
    cef_string_set(linkStr.c_str(), linkStr.length(), &menuInfo.linkUrl, false);
    cef_string_set(imageStr.c_str(), imageStr.length(), &menuInfo.imageUrl,
        false);
    cef_string_set(pageStr.c_str(), pageStr.length(), &menuInfo.pageUrl, false);
    cef_string_set(frameStr.c_str(), frameStr.length(), &menuInfo.frameUrl,
        false);
    cef_string_set(selectedTextStr.c_str(), selectedTextStr.length(),
        &menuInfo.selectionText, false);
    cef_string_set(misspelledWordStr.c_str(), misspelledWordStr.length(),
        &menuInfo.misspelledWord, false);
    menuInfo.editFlags = edit_flags;
    cef_string_set(securityInfoStr.c_str(), securityInfoStr.length(),
        &menuInfo.securityInfo, false);
   

	std::vector<std::string> customItemLabel;
	std::vector<cef_handler_menuitem_t> customItems;


  for (size_t x=0; x<data.customItems.size(); x++) {
	  WebKit::WebMenuItemInfo info = data.customItems[x];

	  std::string label = CefStringUTF8(info.label);

		if (label.size() == 0 && info.type != WebKit::WebMenuItemInfo::Separator)
			continue;

		cef_handler_menuitem_t item;

		item.action = info.action;

		switch (info.type) {
		case WebKit::WebMenuItemInfo::Option:
			item.type = MENUITEMTYPE_OPTION;
			break;

		case WebKit::WebMenuItemInfo::CheckableOption:
			item.type = MENUITEMTYPE_CHECKABLEOPTION;
			break;

		case WebKit::WebMenuItemInfo::Separator:
			item.type = MENUITEMTYPE_SEPERATOR;
			break;

		case WebKit::WebMenuItemInfo::Group:
			item.type = MENUITEMTYPE_GROUP;
			break;

		default:
			continue;
		}

		item.enabled = info.enabled;
		item.checked = info.checked;

		customItemLabel.push_back(label);

		cef_string_utf8_set(customItemLabel.back().c_str(), customItemLabel.back().size(), &item.label, 0);
		customItems.push_back(item);
  }

    menuInfo.customSize = customItems.size();
    menuInfo.customItems = new cef_handler_menuitem_t[menuInfo.customSize];
  
  for (size_t x=0; x<customItems.size(); x++) {
    menuInfo.customItems[x] = customItems[x];
  }

    // Notify the handler that a context menu is requested
    bool res = handler->OnBeforeMenu(browser_, menuInfo);
	delete [] menuInfo.customItems;
	
	if (res)
      goto end;
  }

  // Build the correct default context menu
  if (type_flags &  MENUTYPE_EDITABLE) {
    menu = gtk_menu_new();
    AddMenuItem(browser_, menu, -1, MENU_ID_UNDO, "Undo",
      !!(edit_flags & MENU_CAN_UNDO), label_list);
    AddMenuItem(browser_, menu, -1, MENU_ID_REDO, "Redo",
      !!(edit_flags & MENU_CAN_REDO), label_list);
    AddMenuSeparator(menu, -1);
    AddMenuItem(browser_, menu, -1, MENU_ID_CUT, "Cut",
      !!(edit_flags & MENU_CAN_CUT), label_list);
    AddMenuItem(browser_, menu, -1, MENU_ID_COPY, "Copy",
      !!(edit_flags & MENU_CAN_COPY), label_list);
    AddMenuItem(browser_, menu, -1, MENU_ID_PASTE, "Paste",
      !!(edit_flags & MENU_CAN_PASTE), label_list);
    AddMenuItem(browser_, menu, -1, MENU_ID_DELETE, "Delete",
      !!(edit_flags & MENU_CAN_DELETE), label_list);
    AddMenuSeparator(menu, -1);
    AddMenuItem(browser_, menu, -1, MENU_ID_SELECTALL, "Select All",
      !!(edit_flags & MENU_CAN_SELECT_ALL), label_list);
  } else if(type_flags & MENUTYPE_SELECTION) {
    menu = gtk_menu_new();
    AddMenuItem(browser_, menu, -1, MENU_ID_COPY, "Copy",
      !!(edit_flags & MENU_CAN_COPY), label_list);
  } else if(type_flags & (MENUTYPE_PAGE | MENUTYPE_FRAME)) {
    menu = gtk_menu_new();
    AddMenuItem(browser_, menu, -1, MENU_ID_NAV_BACK, "Back",
      !!(edit_flags & MENU_CAN_GO_BACK), label_list);
    AddMenuItem(browser_, menu, -1, MENU_ID_NAV_FORWARD, "Forward",
      !!(edit_flags & MENU_CAN_GO_FORWARD), label_list);
    AddMenuSeparator(menu, -1);
    AddMenuItem(browser_, menu, -1, MENU_ID_PRINT, "Print",
      true, label_list);
    AddMenuItem(browser_, menu, -1, MENU_ID_VIEWSOURCE, "View Source",
      true, label_list);
  }

  for (size_t x=0; x<data.customItems.size(); x++) {
	  WebKit::WebMenuItemInfo info = data.customItems[x];

      std::string label = CefStringUTF8(info.label);

		if (label.size() == 0 && info.type != WebKit::WebMenuItemInfo::Separator)
			continue;

		switch (info.type) {
		case WebKit::WebMenuItemInfo::Option:
			AddMenuItem(browser_, menu, -1, (cef_handler_menuid_t)(MENU_ID_CUSTOM+info.action), label.c_str(), info.enabled, label_list);
			break;

		case WebKit::WebMenuItemInfo::CheckableOption:
			AddMenuItem(browser_, menu, -1, (cef_handler_menuid_t)(MENU_ID_CUSTOM+info.action), label.c_str(), info.enabled, label_list);
			break;

		case WebKit::WebMenuItemInfo::Separator:
			AddMenuSeparator(menu, -1);
			break;
	
		case WebKit::WebMenuItemInfo::Group:
			AddMenuSeparator(menu, -1);
			break;
		}
  }


  if (menu)
   gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 3, gtk_get_current_event_time());

end:
  MessageLoop::current()->SetNestableTasksAllowed(old_state);
}

void BrowserWebViewDelegate::handleContextMenu(int selected_id)
{
    if (selected_id != 0) {
    
      CefRefPtr<CefClient> client = browser_->GetClient();
      CefRefPtr<CefMenuHandler> handler;
      if (client.get())
        handler = client->GetMenuHandler();
    
      // An action was chosen
      cef_handler_menuid_t menuId =
          static_cast<cef_handler_menuid_t>(selected_id);
      bool handled = false;
      if (handler.get()) {
        // Ask the handler if it wants to handle the action
        handled = handler->OnMenuAction(browser_, menuId);
      }

      if(!handled) {
        // Execute the action
        browser_->UIT_HandleAction(menuId, browser_->GetFocusedFrame());
      }

	  if (!handled) {
		browser_->UIT_GetWebViewHost()->webview()->performCustomContextMenuAction(menuId - MENU_ID_CUSTOM);
	  }
    }
}

// WebWidgetClient ------------------------------------------------------------

void BrowserWebViewDelegate::show(WebNavigationPolicy policy) {
  WebWidgetHost* host = GetWidgetHost();
  if (!host)
  	return;
  	
  GtkWidget* drawing_area = host->view_handle();
  if (!drawing_area)
  	return;
  	
  GtkWidget* window = gtk_widget_get_parent(gtk_widget_get_parent(drawing_area));
  	
  if (window)
  	gtk_widget_show_all(window);
}

void BrowserWebViewDelegate::didChangeCursor(const WebCursorInfo& cursor_info) {
  current_cursor_.InitFromCursorInfo(cursor_info);
  GdkCursorType cursor_type =
      static_cast<GdkCursorType>(current_cursor_.GetCursorType());
  GdkCursor* gdk_cursor;
  if (cursor_type == GDK_CURSOR_IS_PIXMAP) {
    // TODO(port): WebKit bug https://bugs.webkit.org/show_bug.cgi?id=16388 is
    // that calling gdk_window_set_cursor repeatedly is expensive.  We should
    // avoid it here where possible.
    gdk_cursor = current_cursor_.GetCustomCursor();
  } else {
    // Optimize the common case, where the cursor hasn't changed.
    // However, we can switch between different pixmaps, so only on the
    // non-pixmap branch.
    if (cursor_type_ == cursor_type)
      return;
    if (cursor_type == GDK_LAST_CURSOR)
      gdk_cursor = NULL;
    else
      gdk_cursor = gfx::GetCursor(cursor_type);
  }
  cursor_type_ = cursor_type;
  gdk_window_set_cursor(browser_->UIT_GetWebViewWndHandle()->window, gdk_cursor);
}

WebRect BrowserWebViewDelegate::windowRect() {
  // TODO(fix)
  // In full screen mode, this positions the webwidget 4px to the left, and 1px
  // too high. Mouse functions on large <select>'s work fine though.
  // In restored mode, thks positions the webwidget perfectly (with my WM at
  // least), although mouse positions on long <select>'s are off by the offset
  // of the webwidget (sometimes webkit draws it starting from the top of the
  // host) - Keith
  
  if (WebWidgetHost* host = GetWidgetHost()) {
    GtkWidget* drawing_area = host->view_handle();
    
    if (!drawing_area)
      return WebRect();
      
    GtkWidget* next = gtk_widget_get_parent(drawing_area);
    
    if (!next)
      return WebRect();
      
    gint x, x1, y, y1;
    
    x = drawing_area->allocation.x;
    y = drawing_area->allocation.y + 25;
    
    do {
      x += next->allocation.x; 
      y += next->allocation.y;
    
      next = gtk_widget_get_parent(next);
    } while (next && (next != browser_->UIT_GetMainWndHandle()));
    
    if (next) // top window
    {
      gtk_window_get_position(GTK_WINDOW(next), &x1, &y1);
      x += x1;
      y += y1;
    }

  return WebRect(x, y,
                 drawing_area->allocation.width,
                 drawing_area->allocation.height);
  }
  return WebRect();
  
/*  WebWidgetHost* host = GetWidgetHost();
  GtkWidget* drawing_area = host->view_handle();
  GtkWidget* vbox = gtk_widget_get_parent(drawing_area);
//GtkWidget* window = gtk_widget_get_parent(vbox);
  GtkWidget* window = browser_->UIT_GetMainWndHandle();

  gint x, y;
  gtk_window_get_position(GTK_WINDOW(window), &x, &y);
  x += vbox->allocation.x + drawing_area->allocation.x;
  y += vbox->allocation.y + drawing_area->allocation.y;

  return WebRect(x, y,
                 drawing_area->allocation.width,
                 drawing_area->allocation.height);*/
}

void BrowserWebViewDelegate::setWindowRect(const WebRect& rect) {
  if (this == browser_->UIT_GetWebViewDelegate()) {
    // TODO(port): Set the window rectangle.
  } else if (this == browser_->UIT_GetPopupDelegate()) {
    WebWidgetHost* host = GetWidgetHost();
    if (!host)
    	return;
    	
    GtkWidget* drawing_area = host->view_handle();
    if (!drawing_area)
      return;
    	
    GtkWidget* window = gtk_widget_get_parent(gtk_widget_get_parent(drawing_area));
    if (!window)
      return;
    	
    gtk_window_resize(GTK_WINDOW(window), rect.width, rect.height);
    gtk_window_move(GTK_WINDOW(window), rect.x, rect.y);
  }
}

WebRect BrowserWebViewDelegate::rootWindowRect() {
  if (GtkWidget* window = browser_->UIT_GetMainWndHandle()) {
    // We are being asked for the x/y and width/height of the entire browser
    // window.  This means the x/y is the distance from the corner of the
    // screen, and the width/height is the size of the entire browser window.
    // For example, this is used to implement window.screenX and window.screenY.
    gint x, y, width, height;
    
    gtk_window_get_position(GTK_WINDOW(window), &x, &y);
    gtk_window_get_size(GTK_WINDOW(window), &width, &height);
    return WebRect(x, y, width, height);
  }
  return WebRect();
}

WebRect BrowserWebViewDelegate::windowResizerRect() {
  // Not necessary on Linux.
  return WebRect();
}

void BrowserWebViewDelegate::startDragging(
    const WebDragData& data,
    WebDragOperationsMask mask,
    const WebImage& image,
    const WebPoint& image_offset) {
  browser_->UIT_GetWebView()->dragSourceSystemDragEnded();
//  NOTIMPLEMENTED();
}

void BrowserWebViewDelegate::runModal() {
  NOTIMPLEMENTED();
}

// WebPluginPageDelegate ------------------------------------------------------

webkit::npapi::WebPluginDelegate* BrowserWebViewDelegate::CreatePluginDelegate(
    const FilePath& path,
    const std::string& mime_type) {
  // TODO(evanm): we probably shouldn't be doing this mapping to X ids at
  // this level.
  GdkNativeWindow plugin_parent =
      GDK_WINDOW_XWINDOW(browser_->UIT_GetWebViewHost()->view_handle()->window);

  return webkit::npapi::WebPluginDelegateImpl::Create(path, mime_type,
      plugin_parent);
}

void BrowserWebViewDelegate::CreatedPluginWindow(
    gfx::PluginWindowHandle id) {
  browser_->UIT_GetWebViewHost()->CreatePluginContainer(id);
}

void BrowserWebViewDelegate::WillDestroyPluginWindow(
    gfx::PluginWindowHandle id) {
  browser_->UIT_GetWebViewHost()->DestroyPluginContainer(id);
}

void BrowserWebViewDelegate::DidMovePlugin(
    const webkit::npapi::WebPluginGeometry& move) {
  WebWidgetHost* host = GetWidgetHost();
  webkit::npapi::GtkPluginContainerManager* plugin_container_manager =
      static_cast<WebViewHost*>(host)->plugin_container_manager();
  plugin_container_manager->MovePluginContainer(move);
}

// Protected methods ----------------------------------------------------------

void BrowserWebViewDelegate::ShowJavaScriptAlert(
    WebKit::WebFrame* webframe, const CefString& message) {
	GtkWidget* dialog = gtk_message_dialog_new(
	  GTK_WINDOW(browser_->UIT_GetMainWndHandle()), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
	  GTK_BUTTONS_OK, "%s", message.c_str());
	gtk_window_set_title(GTK_WINDOW(dialog), "JavaScript Alert");
	gtk_dialog_run(GTK_DIALOG(dialog));  // Runs a nested message loop.
	gtk_widget_destroy(dialog);
}

bool BrowserWebViewDelegate::ShowJavaScriptConfirm(
    WebKit::WebFrame* webframe, const CefString& message) {
	GtkWidget* dialog = gtk_message_dialog_new(
	  GTK_WINDOW(browser_->UIT_GetMainWndHandle()), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
	  GTK_BUTTONS_OK_CANCEL, "%s", message.c_str());
	gtk_window_set_title(GTK_WINDOW(dialog), "JavaScript Confirm");
	int ret = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	return (ret == GTK_RESPONSE_OK);
}

bool BrowserWebViewDelegate::ShowJavaScriptPrompt(
    WebKit::WebFrame* webframe, const CefString& message,
    const CefString& default_value, CefString* result) {
  NOTIMPLEMENTED();
  return false;
}

static void parseGSList(gpointer data, gpointer user_data) {
  if (!data || !user_data)
    return;
    
  std::vector<FilePath> *file_names = (std::vector<FilePath>*)user_data;
  
  file_names->push_back(FilePath((char*)data));
  g_free(data);
}

// Called to show the file chooser dialog.
bool BrowserWebViewDelegate::ShowFileChooser(std::vector<FilePath>& file_names,
                                             const bool multi_select,
                                             const WebKit::WebString& title,
                                             const FilePath& default_file) {
  GtkWidget *dialog;
  dialog = gtk_file_chooser_dialog_new ("SelectFile",
				      GTK_WINDOW(browser_->UIT_GetMainWndHandle()),
				      GTK_FILE_CHOOSER_ACTION_OPEN,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				      NULL);
  gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), (gboolean)multi_select);
  
  if (!default_file.empty())
    gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), default_file.MaybeAsASCII().c_str());
  
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    if (multi_select) {
      GSList *selected_files = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
      g_slist_foreach(selected_files, parseGSList, (gpointer)&file_names);
      g_slist_free(selected_files);
    } else {
      file_names.push_back(FilePath(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog))));
    }
  } else {
    gtk_widget_destroy(dialog);
    return false;
  }
  
  gtk_widget_destroy(dialog);
  return true;
}
