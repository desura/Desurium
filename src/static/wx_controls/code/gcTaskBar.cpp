/*
Desura is the leading indie game distribution platform
Copyright (C) 2011 Mark Chandler (Desura Net Pty Ltd)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*/

#include "Common.h"
#include "gcTaskBar.h"

#include "wx/dynlib.h"
#include "wx_controls/gcFrame.h"

#ifdef WIN32
#define GetHwndOf(win)          ((HWND)((win)->GetHWND()))
#define GetHiconOf(icon)        ((HICON)(icon).GetHICON())

static UINT gs_msgTaskbar = 0;
static UINT gs_msgRestartTaskbar = 0;
#endif

#ifdef NIX
#include <libnotify/notify.h>
#include <branding/branding.h>
#endif

class gcTaskBarIconWindow : public wxGuiDelegateImplementation<wxFrame>
{
public:
#ifdef WIN32
	gcTaskBarIconWindow(gcTaskBarIcon *icon) : wxGuiDelegateImplementation<wxFrame>(NULL, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0), m_icon(icon)
	{
		// Taskbar restart msg will be sent to us if the icon needs to be redrawn
		gs_msgRestartTaskbar = RegisterWindowMessage(wxT("TaskbarCreated"));

		// Also register the taskbar message here
		gs_msgTaskbar = ::RegisterWindowMessage(wxT("wxTaskBarIconMessage"));
	}

	WXLRESULT MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam)
	{
		bool processed = false;
		WXLRESULT res = m_icon->MSWWindowProc(msg, wParam, lParam, processed);

		if (processed)
			return res;

		if (msg == gs_msgRestartTaskbar || msg == gs_msgTaskbar)
		{
			return m_icon->DefaultWindowProc(msg, wParam, lParam);
		}
		else
		{
			return wxFrame::MSWWindowProc(msg, wParam, lParam);
		}
	}
#endif

private:
	gcTaskBarIcon *m_icon;
};


#ifdef WIN32
static BOOL wxShellNotifyIcon(DWORD dwMessage, NOTIFYICONDATA *pData)
{
	return Shell_NotifyIcon(dwMessage, pData);
}

struct NotifyIconData : public NOTIFYICONDATA
{
	NotifyIconData(WXHWND hwnd)
	{
		memset(this, 0, sizeof(NOTIFYICONDATA));
		cbSize = NOTIFYICONDATA_V3_SIZE;
		hWnd = (HWND) hwnd;
		uCallbackMessage = gs_msgTaskbar;
		uFlags = NIF_MESSAGE;

		// we use the same id for all taskbar icons as we don't need it to
		// distinguish between them
		uID = 99;
	}
};
#endif

gcTaskBarIcon::gcTaskBarIcon() : wxTaskBarIcon()
{
#ifdef WIN32
	this->m_win = new gcTaskBarIconWindow(this);

	static bool s_registered = false;

	if ( !s_registered )
	{
		// Taskbar restart msg will be sent to us if the icon needs to be redrawn
		gs_msgRestartTaskbar = RegisterWindowMessage(wxT("TaskbarCreated"));

		// Also register the taskbar message here
		gs_msgTaskbar = ::RegisterWindowMessage(wxT("wxTaskBarIconMessage"));

		s_registered = true;
	}
	
	m_pEvents = m_win;
#else
	m_pEvents = new gcFrame(NULL, wxID_ANY);
	m_pEvents->Show(false);

	m_pEvents->Bind(wxEVT_CLOSE_WINDOW, &gcTaskBarIcon::onEventClose, this);

	notify_init(PRODUCT_NAME);
#endif
}

gcTaskBarIcon::~gcTaskBarIcon()
{
#ifdef NIX
	if (m_pEvents)
	{
		m_pEvents->Unbind(wxEVT_CLOSE_WINDOW, &gcTaskBarIcon::onEventClose, this);
		m_pEvents->Close();
	}
	
	notify_uninit();
#endif
}

#ifdef WIN32

WXLRESULT gcTaskBarIcon::MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam, bool &processed)
{
	processed = false;
	return 0;
}

long gcTaskBarIcon::DefaultWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam)
{
	return WindowProc(msg, wParam, lParam);
}
#else
void gcTaskBarIcon::onEventClose(wxCloseEvent& event)
{
	wxGuiDelegateImplementation<wxFrame>* frame = dynamic_cast<wxGuiDelegateImplementation<wxFrame>*>(m_pEvents);
	
	if (frame)
		frame->cleanUpEvents();

	m_pEvents = NULL;
	event.Skip();
}

#endif

void gcTaskBarIcon::registerDelegate(wxDelegate* d)
{
	if (!m_pEvents)
	{
		d->nullObject();
		return;
	}

	wxGuiDelegateImplementation<wxFrame>* frame = dynamic_cast<wxGuiDelegateImplementation<wxFrame>*>(m_pEvents);

	if (frame)
		frame->registerDelegate(d);
	else
		d->nullObject();
}

void gcTaskBarIcon::deregisterDelegate(wxDelegate* d)
{
	if (!m_pEvents)
	{
		d->nullObject();
		return;
	}

	wxGuiDelegateImplementation<wxFrame>* frame = dynamic_cast<wxGuiDelegateImplementation<wxFrame>*>(m_pEvents);

	if (frame)
		frame->deregisterDelegate(d);
	else
		d->nullObject();
}

wxEvtHandler* gcTaskBarIcon::GetEventHandler()
{
	if (!m_pEvents)
		return NULL;

	return m_pEvents->GetEventHandler();
}

int gcTaskBarIcon::GetId()
{
	if (!m_pEvents)
		return 0;

	return m_pEvents->GetId();
}

// Operations
bool gcTaskBarIcon::SetIcon(const wxIcon& icon, const wxString& tooltip)
{
#ifdef WIN32
	m_icon = icon;
	m_strTooltip = tooltip;

	NotifyIconData notifyData(GetHwndOf(m_win));

	if (icon.Ok())
	{
		notifyData.uFlags |= NIF_ICON;
		notifyData.hIcon = GetHiconOf(icon);
	}

	// set NIF_TIP even for an empty tooltip: otherwise it would be impossible
	// to remove an existing tooltip using this function
	notifyData.uFlags |= NIF_TIP;
	if ( !tooltip.empty() )
	{
		wxStrlcpy(notifyData.szTip, tooltip.wx_str(), WXSIZEOF(notifyData.szTip));
	}

	bool ok = wxShellNotifyIcon(m_iconAdded ? NIM_MODIFY
											: NIM_ADD, &notifyData) != 0;

	if ( !ok )
	{
		wxLogLastError(wxT("wxShellNotifyIcon(NIM_MODIFY/ADD)"));
	}

	if ( !m_iconAdded && ok )
		m_iconAdded = true;

	return ok;
#else
	return wxTaskBarIcon::SetIcon(icon, tooltip);
#endif
}

bool gcTaskBarIcon::ShowBalloon(const wxString& title, const wxString& text, unsigned msec, int flags)
{
#if defined(WIN32)
	wxCHECK_MSG( m_iconAdded, false,
					_T("can't be used before the icon is created") );

	const HWND hwnd = GetHwndOf(m_win);

	// we need to enable version 5.0 behaviour to receive notifications about
	// the balloon disappearance
	NotifyIconData notifyData(hwnd);
	notifyData.uFlags = 0;
	notifyData.uVersion = 3 /* NOTIFYICON_VERSION for Windows XP */;

	if ( !wxShellNotifyIcon(NIM_SETVERSION, &notifyData) )
	{
		wxLogLastError(wxT("wxShellNotifyIcon(NIM_SETVERSION)"));
	}

	// do show the balloon now
	notifyData = NotifyIconData(hwnd);
	notifyData.uFlags |= NIF_INFO;
	notifyData.uTimeout = msec;
	wxStrlcpy(notifyData.szInfo, text.wx_str(), WXSIZEOF(notifyData.szInfo));
	wxStrlcpy(notifyData.szInfoTitle, title.wx_str(),
				WXSIZEOF(notifyData.szInfoTitle));

	if ( flags & wxICON_INFORMATION )
		notifyData.dwInfoFlags |= NIIF_INFO;
	else if ( flags & wxICON_WARNING )
		notifyData.dwInfoFlags |= NIIF_WARNING;
	else if ( flags & wxICON_ERROR )
		notifyData.dwInfoFlags |= NIIF_ERROR;

	bool ok = wxShellNotifyIcon(NIM_MODIFY, &notifyData) != 0;
	if ( !ok )
	{
		wxLogLastError(wxT("wxShellNotifyIcon(NIM_MODIFY)"));

	}
	return ok;
#elif defined(NIX)
	const char* icon = NULL;
	
	if ( flags & wxICON_INFORMATION )
		icon = "dialog-information";
	else if ( flags & wxICON_WARNING )
		icon = "dialog-warning";
	else if ( flags & wxICON_ERROR )
		icon = "dialog-error";
	
	NotifyNotification* notification =
		notify_notification_new(
			title.c_str(),
			text.c_str(),
			icon);
	
	notify_notification_show(notification, NULL);
	
	return true;
#else
	return false;
#endif
}


bool gcTaskBarIcon::RemoveIcon()
{
#ifdef WIN32
	if (!m_iconAdded)
		return false;

	m_iconAdded = false;

	NotifyIconData notifyData(GetHwndOf(m_win));

	bool ok = wxShellNotifyIcon(NIM_DELETE, &notifyData) != 0;
	if ( !ok )
	{
		wxLogLastError(wxT("wxShellNotifyIcon(NIM_DELETE)"));
	}

	return ok;
#else
	return false;
#endif
}
