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

#ifndef DESURA_GCWEBCONTROL_H
#define DESURA_GCWEBCONTROL_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"
#include "gcWebControlI.h"




namespace ChromiumDLL
{
	class ChromiumBrowserI;
	class JavaScriptContextI;
}

class EventHandler;
class ChromiumMenuInfoFromMem;

//unused is because gcWebHost is typedef as this under linux

class gcWebControl : public gcPanel, public gcWebControlI
{
public:
	gcWebControl(wxWindow* parent, const char* defaultUrl, const char* unused="");
	~gcWebControl();

	virtual void loadUrl(const gcString& url);
	virtual void loadString(const gcString& string);
	virtual void executeJScript(const gcString& code);

	virtual bool refresh();
	virtual bool stop();
	virtual bool back();
	virtual bool forward();

	virtual void forceResize();
	virtual void home();

	Event<int32> onContextSelectEvent;

#ifdef WIN32
	HWND getBrowserHWND();
#endif

	ChromiumDLL::JavaScriptContextI* getJSContext();

	virtual void AddPendingEvent(const wxEvent &event)
	{
		gcPanel::AddPendingEvent(event);
	}
	
	virtual void PopupMenu(wxMenu* menu)
	{
		gcPanel::PopupMenu(menu);
	}

	void showInspector();

protected:
	void onResize(wxSizeEvent& event);
	void onPaintBg(wxEraseEvent& event);
	void onPaint(wxPaintEvent& event);

	void onMenuClicked(wxCommandEvent& event);
	void onMouseScroll(wxMouseEvent& event);
	void onFocus(wxFocusEvent& event);

	void onStartLoad();
	void onPageLoad();
	virtual void handleKeyEvent(int eventCode);
	
	friend class EventHandler;

private:
	ChromiumDLL::ChromiumBrowserI* m_pChromeBrowser;
	bool m_bStartedLoading;
	bool m_bContentLoaded;

	gcString m_szHomeUrl;

	EventHandler* m_pEventHandler;
};

#endif //DESURA_GCWEBCONTROL_H
