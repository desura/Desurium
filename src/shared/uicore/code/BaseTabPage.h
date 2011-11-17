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

#ifndef DESURA_BASETABPAGE_H
#define DESURA_BASETABPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcControls.h"

class BaseToolBarControl;

class baseTabPage : public gcPanel
{
public:
	baseTabPage( wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize( 500,300 ), long style = 0 );
	~baseTabPage();

	void setProgress(const char* caption, int32 prog);

	Event<uint32> progressUpdateEvent;

	int32 getProg(){return m_iProgress;}
	
	bool showProg(){return m_bShowProg;}

	virtual void setSelected(bool state);
	virtual BaseToolBarControl* getToolBarControl()=0;

	const char* getProgCap()
	{
		return m_szProgCap.c_str();
	}

	virtual void reset()
	{
	}

protected:

private:
	gcWString m_wszSearch;
	gcString m_szProgCap;

	int32 m_iProgress;
	bool m_bShowProg;
};

#endif //DESURA_BASETABPAGE_H
