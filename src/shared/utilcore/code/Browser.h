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

#ifndef DESURA_GCWEBCONTROL_H
#define DESURA_GCWEBCONTROL_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"

extern bool InitWebControl();
extern void ShutdownWebControl();

namespace ChromiumDLL
{
	class ChromiumBrowserI;
}

class EventHandler;

class Browser : public gcPanel
{
public:
    Browser(wxWindow* parent);
	~Browser();

	void setCookie(const char* name, const char* value);
	void loadUrl(const char* url);

protected:
	void onResize( wxSizeEvent& event );
	void onPaintBg( wxEraseEvent& event );
	void onPaint( wxPaintEvent& event );

	friend class EventHandler;

private:
	ChromiumDLL::ChromiumBrowserI* m_pChromeBrowser;
	EventHandler* m_pEventHandler;

	DECLARE_EVENT_TABLE();
};

#endif //DESURA_GCWEBCONTROL_H
