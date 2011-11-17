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
#include "MainFormCustomFrame.h"

#include "Managers.h"

#ifdef NIX
#include <gtk/gtk.h>
#endif

gcMainCustomFrameImpl::gcMainCustomFrameImpl(wxTopLevelWindow* parent, FrameIcon* frameIcon, long style) : gcCustomFrameImpl(parent, frameIcon, style)
{
	m_imgMainBorder = GetGCThemeManager()->getImageHandle("#main_header_border");
	m_imgMainBorderNA = GetGCThemeManager()->getImageHandle("#main_header_border_nonactive");
	m_pLastEventHandler = NULL;

	setTitleHeight(0);
	showTitle(false);

	m_bMenuHover = false;
	safe_delete(m_pCFButtons);
}

gcMainCustomFrameImpl::~gcMainCustomFrameImpl()
{
}

bool gcMainCustomFrameImpl::handleEvent(wxEvtHandler *obj, wxMouseEvent &event)
{
	if (!event.GetEventObject())
		return false;

	if (event.GetEventType() == wxEVT_LEFT_DCLICK)
	{
		onLeftDClick(event);
	}
	else if (event.GetEventType() == wxEVT_MOTION)
	{
		onMouseMove(event);
	}
	else if (event.GetEventType() == wxEVT_LEFT_DOWN)
	{
		onLeftMouseDown(event);
		m_pLastEventHandler = obj;
	}
	else if (event.GetEventType() == wxEVT_LEFT_UP)
	{
		onLeftMouseUp(event);
	}
	else if (event.GetEventType() == wxEVT_RIGHT_DOWN)
	{
		onRightMouseDown(event);
	}

	bool skipped = event.GetSkipped();
	event.Skip(false);

	return !skipped;
}

void gcMainCustomFrameImpl::onLeftMouseUp(wxMouseEvent& event)
{
	gcCustomFrameImpl::onLeftMouseUp(event);

	if (event.GetSkipped())
	{
		wxObject *obj = event.GetEventObject();

		//Normally event is due to CaptureMouse so make sure if a control gave us the mouse down we give it mouse up
		if (obj == m_pParent && m_pLastEventHandler)
			m_pLastEventHandler->AddPendingEvent(event);
	}

	m_pLastEventHandler = NULL;
}

void gcMainCustomFrameImpl::calcBorders()
{
	gcCustomFrameImpl::calcBorders();
	m_TitleBorder.SetHeight(40);
	m_pCFMove->calcBorders(m_TitleBorder, m_TitleAreaRect);
}

void gcMainCustomFrameImpl::generateBorder(wxDC* dc)
{
	wxRect wbr = getWindowsBorderRect();

	if (!m_imgMainBorder->IsOk() || m_pParent->IsMaximized())
	{
		gcCustomFrameImpl::generateBorder(dc);
		return;
	}

	int ih = m_imgMainBorder->GetSize().GetHeight();
	int iw = m_imgMainBorder->GetSize().GetWidth();

	wxImage borderImg = isActive()?m_imgMainBorder->Scale(iw, ih):m_imgMainBorderNA->Scale(iw, ih);
	wxBitmap borderBMPR(borderImg);

	int w = wbr.GetWidth() + wbr.GetLeft() - borderBMPR.GetWidth();
	wxRect imgRect(wxPoint(w, wbr.GetTop()), borderBMPR.GetSize());

	wxRegion region = generateBorderRegion();
	region.Subtract(imgRect);

	dc->DestroyClippingRegion();
	dc->SetDeviceClippingRegion(region);

	gcCustomFrameImpl::generateBorder(dc);

	dc->DestroyClippingRegion();

	dc->DrawBitmap(borderBMPR, imgRect.GetTopLeft(), true);
}

void gcMainCustomFrameImpl::generateTitle(wxDC* dc)
{
}

void gcMainCustomFrameImpl::DoGetClientSize(int *width, int *height) const
{
	gcCustomFrameImpl::DoGetClientSize(width, height);

	if (isMaximized() == false)
		*width += 5; //missing left border

	*height += 7; //missing top border
}

wxPoint gcMainCustomFrameImpl::GetClientAreaOrigin() const
{
	if (isMaximized())
		return wxPoint(0,0);

	return wxPoint(1,1);
}

void gcMainCustomFrameImpl::setMenuHover(bool state)
{
	m_bMenuHover = state;
	m_pParent->Refresh(false);
}

wxRect gcMainCustomFrameImpl::getClientAreaRect()
{
	wxRect wbr = getWindowsBorderRect();
	wxRect cr = m_pParent->GetClientRect();
	
	return wxRect(wbr.GetTopLeft(), cr.GetSize());
}

void gcMainCustomFrameImpl::onPaintNC(wxNcPaintEvent& event)
{
	wxWindowDC dc(m_pParent);

	wxSize pSize = m_pParent->GetSize();

	wxRect ca = getClientAreaRect();
	ca.x += 1;
	ca.y += 1;
	ca.width -= 1;
	ca.height -= 1;

	wxRegion region(wxRect(wxPoint(0,0), pSize));
	region.Subtract(ca);

	if (!m_BorderBitmap.IsOk() || m_BorderBitmap.GetSize() != pSize)
		m_BorderBitmap = wxBitmap(pSize);

	wxMemoryDC tmpDC(m_BorderBitmap);
	tmpDC.DestroyClippingRegion();
	tmpDC.SetDeviceClippingRegion(region);

	doPaintBorder(&tmpDC);
	tmpDC.SelectObject(wxNullBitmap);

	dc.DestroyClippingRegion();
	dc.SetDeviceClippingRegion(region);

	dc.DrawBitmap(m_BorderBitmap, wxPoint(0,0), true);
}

void gcMainCustomFrameImpl::onPaintBG( wxEraseEvent& event )
{
	wxDC* dc = event.GetDC();

	wxRect ca = getClientAreaRect();
	wxColor col = m_pParent->GetBackgroundColour();

	ca.x += 1;
	ca.y += 1;
	ca.width -= 1;
	ca.height -= 1;

	dc->DestroyClippingRegion();
	dc->SetDeviceClippingRegion(ca);

	dc->SetPen(wxPen(col));
	dc->SetBrush(wxBrush(col));
	dc->DrawRectangle(ca);

	dc->DestroyClippingRegion();


	//wxSize pSize = m_pParent->GetSize();
	//wxBitmap tmp(pSize);

	//wxMemoryDC tmpDC(tmp);
	//tmpDC.DestroyClippingRegion();
	//tmpDC.SetDeviceClippingRegion(ca);
	//tmpDC.SetPen(wxPen(col));
	//tmpDC.SetBrush(wxBrush(col));
	//tmpDC.DrawRectangle(ca);
	//tmpDC.SelectObject(wxNullBitmap);

	//tmp.SaveFile("bg_paint.bmp", wxBitmapType::wxBITMAP_TYPE_BMP);

	wxNcPaintEvent ncE;
	onPaintNC(ncE);

	m_pParent->Refresh(false);
}

void gcMainCustomFrameImpl::doMaximize()
{
	if (m_bIsMaximized)
		return;

	m_bIsMaximized = true;
	m_RestorSize = m_pParent->GetRect();

#ifdef WIN32
	MONITORINFO mi = { sizeof(mi) };

	HWND hwnd = (HWND)m_pParent->GetHWND();

	WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };
	GetWindowPlacement(hwnd, &g_wpPrev);
	GetMonitorInfo(MonitorFromWindow(hwnd,  MONITOR_DEFAULTTOPRIMARY), &mi);
	wxRect wbr = getWindowsBorderRect();

	wxPoint pos = wxPoint(mi.rcWork.left, mi.rcWork.top) - wbr.GetTopLeft();
	wxSize size = wxSize(mi.rcWork.right - mi.rcWork.left, mi.rcWork.bottom - mi.rcWork.top);
#else
	gint posX;
	gint posY;
	gint winW;
	gint winH;
	gdk_window_get_position(GDK_WINDOW(m_pParent->GetHandle()), &posX, &posY);
	gdk_window_get_size(GDK_DRAWABLE(m_pParent->GetHandle()), &winW, &winH);

	wxRect wbr = getWindowsBorderRect();

	wxPoint pos = wxPoint((int)posX, (int)posY) - wbr.GetTopLeft();
	wxSize size = wxSize((int)winW, (int)winH);
#endif

	m_pParent->SetSize(wxRect(pos, size));

	if (m_pCFButtons)
		m_pCFButtons->invalidateBitmap();

	calcBorders();
	setFrameRegion();
	m_pParent->Refresh(true);
}
