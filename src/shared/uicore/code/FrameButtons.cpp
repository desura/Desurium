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

#include "Common.h"
#include "FrameButtons.h"
#include "MainApp.h"



FrameButtons::FrameButtons(wxWindow *parent, gcFrame* controler) 
	: gcFlickerFreePaint<wxEventDelegate<gcPanel>>(parent), CustomFrameButtons(this, controler, wxCLOSE_BOX|wxMINIMIZE_BOX|wxMAXIMIZE_BOX, "#menu_bg", "#menu_bg_nonactive") 
{
	SetMinSize(getRect().GetSize());

	Bind( wxEVT_LEFT_DCLICK, &FrameButtons::onMouseDClick, this);
	Bind( wxEVT_MOTION, &FrameButtons::onMouseMotion, this);
	Bind( wxEVT_LEFT_DOWN, &FrameButtons::onMouseLDown, this);
	Bind( wxEVT_LEFT_UP, &FrameButtons::onMouseLUp, this);
	Bind( wxEVT_MOUSE_CAPTURE_LOST, &FrameButtons::onMouseCaptureLost, this);

	calcBorders(getRect().GetSize().GetWidth(), 0);
}

FrameButtons::~FrameButtons()
{
}

void FrameButtons::calcBorders(int wb, int yoff)
{
	CustomFrameButtons::calcBorders(wb, yoff);
	m_ButtonPos.y = 0;
}

void FrameButtons::onActiveToggle(bool state)
{
	setActive(state);
	invalidatePaint();
}

void FrameButtons::changeMaxButton(bool isMaxed)
{
	invalidateBitmap();
}

void FrameButtons::onMouseCaptureLost(wxMouseCaptureLostEvent &event)
{
	if (HasCapture())
		ReleaseMouse();

	onMouseUp();
}

void FrameButtons::onMouseDClick( wxMouseEvent& event )
{
	onMouseDoubleClick();
}

void FrameButtons::onMouseMotion( wxMouseEvent& event )
{
	onMouseMove();
}

void FrameButtons::onMouseLDown( wxMouseEvent& event )
{
	onMouseDown();
}

void FrameButtons::onMouseLUp( wxMouseEvent& event )
{
	onMouseUp();
}

void FrameButtons::doPaint(wxDC* dc)
{
	drawBg(dc);
	generateButtons(dc);
}

void FrameButtons::clearBg(wxDC* dc)
{
	drawBg(dc);
}

void FrameButtons::drawBg(wxDC* dc)
{
	wxSize sz = GetSize();

	uint32 w = sz.GetWidth();
	uint32 h = sz.GetHeight();

	wxBitmap borderBMP = isActive()?m_imgBorder->Copy():m_imgBorderNA->Copy();
	wxBitmap temp(w, h);
	gcImage::tileImg(temp, borderBMP);
	
	dc->DrawBitmap(temp, wxPoint(0,0), true);
}

void FrameButtons::invalidatePaint()
{
	CustomFrameButtons::invalidateBitmap();
	gcFlickerFreePaint<wxEventDelegate<gcPanel>>::invalidatePaint();
}

void FrameButtons::invalidateBitmap()
{
	CustomFrameButtons::invalidateBitmap();
	gcFlickerFreePaint<wxEventDelegate<gcPanel>>::invalidatePaint();
}