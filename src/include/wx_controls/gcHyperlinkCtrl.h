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

#ifndef DESURA_GCHYPERLINKCTRL_H
#define DESURA_GCHYPERLINKCTRL_H

#ifdef _WIN32
#pragma once
#endif

#include <wx/hyperlink.h>

class gcHyperlinkCtrl : public wxHyperlinkCtrl
{
public:
	gcHyperlinkCtrl(wxWindow *parent, wxWindowID id, const wxString& label, const wxString& url, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxHL_DEFAULT_STYLE);

	void setUnderlined(bool state);

protected:
	virtual void onFocus(wxFocusEvent& event);
	virtual void onBlur(wxFocusEvent& event);
	virtual void onPaint(wxPaintEvent& event);
	virtual void onChar(wxKeyEvent& event);

	virtual wxSize DoGetBestSize() const;

private:
	void onEraseBackGround(wxEraseEvent& event);

	wxColor m_NormalColor;
	bool m_bUnderlined;
};

#endif //DESURA_GCHYPERLINKCTRL_H
