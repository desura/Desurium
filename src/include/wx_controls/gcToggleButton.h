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

#ifndef DESURA_GCTOGGLEBUTTON_H
#define DESURA_GCTOGGLEBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include "gcButton.h"

//! A button that can toggle on or off
class gcToggleButton : public gcButton
{
public:
	gcToggleButton( wxWindow* parent, wxWindowID id = wxID_ANY, const wxString& text = wxString(""), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxNO_BORDER|wxTAB_TRAVERSAL );
	~gcToggleButton();

	bool GetValue(){return m_bToggled;}
	void SetValue(bool state=false){m_bToggled=state;}

protected:
	virtual STATE getState();
	virtual void setImage();

	void onMouseDown(wxMouseEvent& event);

private:
	const char* m_szToggleOn;
	const char* m_szToggleOff;

	bool m_bToggled;
	wxColour m_TextColor;
};


#endif //DESURA_GCTOGGLEBUTTON_H
