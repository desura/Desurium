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

#ifndef DESURA_GCCUSTOMFRAMEIMPL_H
#define DESURA_GCCUSTOMFRAMEIMPL_H
#ifdef _WIN32
#pragma once
#endif

#include <wx/wx.h>

#include "gcImageHandle.h"

#include "gcCustomFrameButtons.h"
#include "gcCustomFrameResize.h"
#include "gcCustomFrameMove.h"

class gcFrame;
class gcImageButton;
class FrameIcon;

enum MOUSE_ACTION
{
	MOUSE_MOVE,
	MOUSE_LDOWN,
	MOUSE_LUP,
};

enum BORDER_DIR
{
	NORTH = 0,
	NEAST,
	EAST,
	SEAST,
	SOUTH,
	SWEST,
	WEST,
	NWEST,
	BORDER_DIR_MAX,
};

//! Custom frame implementation 
class gcCustomFrameImpl
{
public:
	gcCustomFrameImpl(wxTopLevelWindow* parent, FrameIcon* frameIcon, long style);
	~gcCustomFrameImpl();

	virtual void DoGetClientSize(int *width, int *height) const;
	virtual wxPoint GetClientAreaOrigin() const;

	void setTitleHeight(unsigned int height);
	void showTitle(bool state);

	bool isResizing();

	virtual void invalidatePaint()
	{
		m_bBGBitMapValid = false;
		m_pParent->Refresh(false);
	}

	bool isActive() const
	{
		return m_bActive;
	}

	bool isMaximized() const
	{
		return m_bIsMaximized;
	}

	virtual void maximize(bool state);
	virtual void minimize(bool state);
	virtual void restore();

#ifdef WIN32
	virtual WXLRESULT PreMSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam, bool &processed);
	virtual WXLRESULT PostMSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam, WXLRESULT res);
#endif


	Event<bool> onActiveEvent;


	void destroy();

protected:
	wxRegion generateBorderRegion();

	virtual void onPaintNC(wxNcPaintEvent& event);
	virtual void onPaintBG( wxEraseEvent& event );
	virtual void onPaint(wxPaintEvent& event);
	virtual void onResize(wxSizeEvent& event);

	virtual void onLeftDClick( wxMouseEvent& event );
	virtual void onLeftMouseDown( wxMouseEvent& event );
	virtual void onLeftMouseUp( wxMouseEvent& event );
	virtual void onRightMouseDown(wxMouseEvent& event);
	virtual void onMouseMove( wxMouseEvent& event );

	virtual void onBlur(wxFocusEvent& event);
	virtual void onMouseCaptureLost(wxMouseCaptureLostEvent &event);

	virtual void calcBorders();

	virtual void doPaintBorder(wxDC* dc);
	virtual void generateTitle(wxDC* dc);
	virtual void generateBorder(wxDC* dc);
	virtual void setFrameRegion();

	wxRect getWindowsBorderRect() const;

	virtual void doMaximize();
	virtual void doRestore();

	wxRect m_RestorSize;
	bool m_bIsMaximized;


	std::vector<wxRect> m_FrameBorder;

	wxRect m_TitleAreaRect;
	wxRect m_TitleBorder;
	wxRect m_ClientArea;
	wxRect m_PaintClientArea;
	wxRect m_IconRect;

	bool m_bShowTitle;
	bool m_bMaxEnabled;
	bool m_bMinEnabled;
	bool m_bActive;

	unsigned int m_iTitleHeight;

	wxTopLevelWindow* m_pParent;
	FrameIcon* m_pFrameIcon;

	bool m_bValidRegion;
	bool m_bBGBitMapValid;
	bool m_bDisableSetRegion;
	bool m_bWasLastMaximised;

	wxSize m_LastSize;




	gcImageHandle m_imgBorder;
	gcImageHandle m_imgBorderNA;

	CustomFrameButtons *m_pCFButtons;
	CustomFrameResize* m_pCFResize;
	CustomFrameMove* m_pCFMove;

	wxBitmap m_BorderBitmap;
};








#endif //DESURA_GCCUSTOMFRAMEIMPL_H
