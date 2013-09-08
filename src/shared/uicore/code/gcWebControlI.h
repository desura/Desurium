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

#ifndef DESURA_GCWEBCONTROLI_H
#define DESURA_GCWEBCONTROLI_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/event.h>

typedef struct
{
	const char* url;
	bool stop;
	bool main;
} newURL_s;

class Crumb
{
public:
	gcString name;
	gcString url;
};

class ChromiumMenuInfoFromMem;

class gcWebControlI
{
public:
	virtual void loadUrl(const gcString& url)=0;
	virtual void loadString(const gcString& string)=0;
	virtual void executeJScript(const gcString& code)=0;

	virtual bool refresh()=0;
	virtual bool stop()=0;
	virtual bool back()=0;
	virtual bool forward()=0;

	virtual void forceResize()=0;
	virtual void home()=0;

	virtual void AddPendingEvent(const wxEvent &event)=0;
	virtual void handleKeyEvent(int eventCode)=0;

	Event<newURL_s> onNewURLEvent;
	EventV onPageLoadEvent;
	EventV onPageStartEvent;
	EventV onAnyPageLoadEvent;

	EventV onClearCrumbsEvent;
	Event<Crumb> onAddCrumbEvent;

	EventV onFindEvent;

	//! Callee is responsable for deleting menu
	virtual void HandlePopupMenu(ChromiumMenuInfoFromMem* menu){};
	virtual void PopupMenu(wxMenu* menu)=0;


#ifdef WIN32
	virtual HWND getBrowserHWND()=0;
#endif
};

#endif //DESURA_GCWEBCONTROLI_H
