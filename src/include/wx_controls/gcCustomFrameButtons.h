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

#ifndef DESURA_GCCUSTOMFRAMEBUTTONS_H
#define DESURA_GCCUSTOMFRAMEBUTTONS_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/wx.h>
#include "gcImageHandle.h"

//! Top frame buttons that handle close, min and max for the custom window frame
class CustomFrameButtons
{
public:
	CustomFrameButtons(wxWindow* pParent, wxTopLevelWindow* pFrame, int style, const char* imgActive, const char* imgNonActive);

	bool onMouseMove();
	bool onMouseDown();
	bool onMouseUp();
	bool onMouseDoubleClick();

	void generateButtons(wxDC* dc);
	wxRect getRect();

	virtual void invalidateBitmap();
	virtual void calcBorders(int dw, int yoff);

	bool isActive();
	void setActive(bool state);

protected:
	int32 isWithingButtons(uint8 action);

	virtual void clearBg(wxDC* dc);

	gcImageHandle m_imgBorder;
	gcImageHandle m_imgBorderNA;

	wxPoint m_ButtonPos;

private:
	std::vector<wxRect> m_ButtonRect;
	std::vector<int32> m_iButtonState;
	
	bool m_bActive;
	bool m_bButtonBitMapValid;
	int32 m_iMouseDown;

	wxBitmap m_ButtonBitMap;
	wxWindow* m_pParent;
	wxTopLevelWindow* m_pFrame;
};

#endif //DESURA_GCCUSTOMFRAMEBUTTONS_H
