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

#ifndef DESURA_GCTIPWINDOW_H
#define DESURA_GCTIPWINDOW_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/popupwin.h>
#include "wx/arrstr.h"

class gcTipWindowView;

class gcTipWindow : public wxPopupTransientWindow
{
public:
    gcTipWindow(wxWindow *parent, const wxString& text, wxCoord maxLength = 100, gcTipWindow** windowPtr = NULL, wxRect *rectBound = NULL);

    virtual ~gcTipWindow();

    void SetTipWindowPtr(gcTipWindow** windowPtr) ;
    void SetBoundingRect(const wxRect& rectBound);

    virtual void Close();
	virtual bool Show(bool show=true);

protected:
    bool CheckMouseInBounds(const wxPoint& pos);
    void OnMouseClick(wxMouseEvent& event);
	void onMouseCaptureLost(wxMouseCaptureLostEvent &event);

    virtual void OnDismiss();
	virtual void OnIdle(wxIdleEvent& event);
	virtual void Popup(wxWindow *winFocus);

private:
    wxArrayString m_textLines;
    wxCoord m_heightLine;

    gcTipWindowView *m_view;
    gcTipWindow** m_windowPtr;

    wxRect m_rectBound;

    friend class gcTipWindowView;
};


#endif //DESURA_GCTIPWINDOW_H
