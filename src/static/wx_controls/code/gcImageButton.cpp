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
#include "Common.h"
#include "gcImageButton.h"


gcImageButton::gcImageButton( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : gcImageControl( parent, id, pos, size, style )
{
	Bind(wxEVT_ENTER_WINDOW, &gcImageButton::onMouseOver, this);
	Bind(wxEVT_LEAVE_WINDOW, &gcImageButton::onMouseOut, this);
	Bind(wxEVT_LEFT_UP, &gcImageButton::onMouseUp, this);
	Bind(wxEVT_LEFT_DOWN, &gcImageButton::onMouseDown, this);
	Bind(wxEVT_RIGHT_UP, &gcImageButton::onMouseUp, this);
	Bind(wxEVT_MOTION, &gcImageButton::onMouseEvent, this);

	Bind(wxEVT_UPDATE_UI, &gcImageButton::onUpdateUI, this);
	Bind(wxEVT_MOUSE_CAPTURE_LOST, &gcImageButton::onMouseCaptureLost, this);

	m_bHovering = false;
	m_bMouseDown = false;
	m_bMouseDownAndOver = false;

	//dont want it for buttons
	setTrans(true);
	m_bStoreRegion = true;
}

gcImageButton::~gcImageButton()
{
	if (HasCapture())
		ReleaseMouse();
}

void gcImageButton::setImage(const char* image)
{
	if (image && m_szCurImage == image)
		return;

	gcImageControl::setImage(image);
	m_szCurImage = image;
}

void gcImageButton::setDefaultImage(const char* image)
{
	bool res = (m_szImage.size() == 0);

	m_szImage = image;

	if (res)
		setImage(m_szImage.c_str());
	else
		refreshImage();
}

void gcImageButton::setFocusImage(const char* image)
{
	m_szFocusImage = image;
	refreshImage();
}

void gcImageButton::setHoverImage(const char* image)
{
	m_szHoverImage = image;
	refreshImage();
}

void gcImageButton::setDisabledImage(const char* image)
{
	m_szDisabledImage = image;
	refreshImage();
}

void gcImageButton::refreshImage()
{
	if (IsEnabled() || m_szDisabledImage.size() == 0)
	{	
		if (m_bMouseDownAndOver && m_szFocusImage.size() > 0)
			setImage(m_szFocusImage.c_str());
		else if (m_bHovering && m_szHoverImage.size() > 0)
			setImage(m_szHoverImage.c_str());
		else
			setImage(m_szImage.c_str());
	}
	else
	{
		setImage(m_szDisabledImage.c_str());
	}
}

bool gcImageButton::Enable(bool enable)
{
	bool val = gcImageControl::Enable(enable);
	refreshImage();
	return val;
}

bool gcImageButton::mouseOverButton( wxMouseEvent& event)
{
	wxPoint pos = event.GetPosition();
	wxRect rect = this->GetRect();

	//need to reset the pos as it might be offseted from parent
	rect.SetPosition(wxPoint(0,0));
	
	wxPoint realPos = pos - rect.GetPosition();
	
	return rect.Contains(pos) && (!m_bTransEnabled || !m_Region.IsOk() || m_Region.Contains(realPos));
}


void gcImageButton::updateMouse()
{
	wxMouseEvent event;

	wxRect panelRec = GetScreenRect(); 
	wxPoint mousePoint = wxGetMousePosition();

	wxPoint realPos = mousePoint - panelRec.GetPosition();

	event.m_x = realPos.x;
	event.m_y = realPos.y;

	updateMouse(event);
}

void gcImageButton::updateMouse(wxMouseEvent& event)
{
	if (!IsEnabled())
		return;

	bool bNeedsUpdate = false;

	if (mouseOverButton(event))
	{
		if (m_bMouseDown && !m_bMouseDownAndOver)
		{
			m_bMouseDownAndOver = true;
			bNeedsUpdate = true;
		}
		else if (!m_bMouseDown && m_bMouseDownAndOver)
		{
			m_bMouseDownAndOver = false;
			bNeedsUpdate = true;
		}

		if (!m_bHovering)
		{
			m_bHovering = true;
			bNeedsUpdate = true;
		}
	}
	else 
	{		
		if (m_bMouseDownAndOver)
		{
			m_bMouseDownAndOver = false;
			bNeedsUpdate = true;
		}

		if (m_bHovering)
		{
			m_bHovering = false;
			bNeedsUpdate = true;
		}
	}

	if (bNeedsUpdate)
		refreshImage();
}

void gcImageButton::onMouseOver( wxMouseEvent& event )
{
	updateMouse(event);
	event.Skip();
}

void gcImageButton::onMouseOut( wxMouseEvent& event )
{
	updateMouse(event);
	event.Skip();
}

void gcImageButton::onMouseUp( wxMouseEvent& event )
{
	bool oldMouse = m_bMouseDown;
	m_bMouseDown = false;

	updateMouse(event);
	event.Skip();

	if (HasCapture())
		ReleaseMouse();

	refreshImage();

	if (mouseOverButton(event) && oldMouse && IsEnabled())
	{
		wxCommandEvent e(wxEVT_COMMAND_BUTTON_CLICKED, GetId());
		e.SetEventObject(this);
		HandleWindowEvent(e);
	}
}

void gcImageButton::onMouseDown( wxMouseEvent& event )
{
	m_bMouseDown = true;
	updateMouse(event);
	refreshImage();

	event.Skip();
}

void gcImageButton::onMouseEvent( wxMouseEvent& event )
{
	if (!m_bMouseDown && !mouseOverButton(event))
	{
		if (HasCapture())
			ReleaseMouse();
	}
	else
	{
		if (!HasCapture())
			CaptureMouse();
	}

	updateMouse(event);
	event.Skip();
}

void gcImageButton::onMouseCaptureLost(wxMouseCaptureLostEvent &event)
{
	if (HasCapture())
		ReleaseMouse();

	m_bMouseDown = false;
	updateMouse();
}

void gcImageButton::onUpdateUI( wxUpdateUIEvent& event )
{
	updateMouse();
	event.Skip();
}

void gcImageButton::doPaint(wxDC* dc)
{
	gcImageControl::doPaint(dc);
	updateMouse();
}
