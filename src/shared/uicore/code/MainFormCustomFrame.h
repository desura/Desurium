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

#ifndef DESURA_MAINFORMCUSTOMFRAME_H
#define DESURA_MAINFORMCUSTOMFRAME_H
#ifdef _WIN32
#pragma once
#endif

#include "wx_controls/gcCustomFrameImpl.h"


class gcMainCustomFrameImpl : public gcCustomFrameImpl
{
public:
	gcMainCustomFrameImpl(wxTopLevelWindow* parent, FrameIcon* frameIcon, long style);
	~gcMainCustomFrameImpl();

	virtual void DoGetClientSize(int *width, int *height) const;
	virtual wxPoint GetClientAreaOrigin() const;

	bool handleEvent(wxEvtHandler *obj, wxMouseEvent &event);
	void setMenuHover(bool state);

protected:
	virtual void calcBorders();
	virtual void generateBorder(wxDC* dc);
	virtual void generateTitle(wxDC* dc);

	virtual void onLeftMouseUp(wxMouseEvent& event);

	virtual void onPaintBG( wxEraseEvent& event );
	virtual void onPaintNC(wxNcPaintEvent& event);

	wxRect getClientAreaRect();

	virtual void doMaximize();

private:
	std::vector<wxEvtHandler*> m_vControlList;

	bool m_bMenuHover;

	gcImageHandle m_imgMainBorder;
	gcImageHandle m_imgMainBorderNA;

	wxEvtHandler* m_pLastEventHandler;
};

#endif //DESURA_MAINFORMCUSTOMFRAME_H
