/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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

#ifndef DESURA_GCTASKBAR_H
#define DESURA_GCTASKBAR_H
#ifdef _WIN32
#pragma once
#endif

#include "wx/taskbar.h"
#include "guiDelegate.h"

//! An icon that shows in the doc or menu bar with a right click menu
class gcTaskBarIcon : public wxTaskBarIcon
{
public:
	gcTaskBarIcon();
	~gcTaskBarIcon();

#ifdef WIN32
	virtual WXLRESULT MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam, bool &processed);
	long DefaultWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam);
#else
	void onEventClose(wxCloseEvent& event);
#endif

	virtual bool SetIcon(const wxIcon& icon, const wxString& tooltip = wxEmptyString);
	virtual bool RemoveIcon();

	bool ShowBalloon(const wxString& title, const wxString& text, unsigned msec = 0, int flags = 0);

	///////////////////////////////////////////////////////
	// guiDelegate overrides
	///////////////////////////////////////////////////////
	virtual void registerDelegate(wxDelegate* d);
	virtual void deregisterDelegate(wxDelegate* d);

	virtual wxEvtHandler* GetEventHandler();
	virtual int GetId();
	
protected:
	wxWindow* m_pEvents;
};


#endif //DESURA_GCTASKBAR_H
