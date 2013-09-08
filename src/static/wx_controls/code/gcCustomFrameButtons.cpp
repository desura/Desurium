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
#include "gcCustomFrameButtons.h"

#include <wx/wx.h>
#include "gcImageHandle.h"

#include "gcCustomFrameImpl.h"
#include "gcThemeManager.h"

enum
{
	BUTTON_CLOSE = 0,
	BUTTON_MAX,
	BUTTON_MIN,
	BUTTON_RESTORE,
};

const char* g_szImageNames[] =
{
	"#main_close",
	"#main_max",
	"#main_min",
	"#main_restore",
};

enum
{
	BSTATE_NORMAL = 0,
	BSTATE_HOVER,
	BSTATE_FOCUSE,
	BSTATE_DISABLED,
};

const char* g_szSpriteNames[] =
{
	"normal",
	"hover",
	"focus",
	"disabled",
};


CustomFrameButtons::CustomFrameButtons(wxWindow* parent, wxTopLevelWindow* pFrame, int style, const char* imgActive, const char* imgNonActive)
{
	m_pParent = parent;
	m_pFrame = pFrame;

	m_iMouseDown = 0;
	m_ButtonRect.resize(3);
	m_iButtonState.resize(3);
	m_bButtonBitMapValid = false;
	m_bActive = true;

	m_ButtonBitMap = wxBitmap(106,22);

	m_iButtonState[BUTTON_CLOSE] = BSTATE_NORMAL;
	m_iButtonState[BUTTON_MAX] = BSTATE_NORMAL;
	m_iButtonState[BUTTON_MIN] = BSTATE_NORMAL;

	m_ButtonRect[BUTTON_MIN] = wxRect(wxPoint(0,0), wxSize(30,22));
	m_ButtonRect[BUTTON_MAX] = wxRect(wxPoint(30,0), wxSize(28,22));
	m_ButtonRect[BUTTON_CLOSE] = wxRect(wxPoint(58,0), wxSize(48,22));
	
	if (!(style & wxCLOSE_BOX))
		m_iButtonState[BUTTON_CLOSE] = BSTATE_DISABLED;

	if (!(style & wxMAXIMIZE_BOX))
		m_iButtonState[BUTTON_MAX] = BSTATE_DISABLED;

	if (!(style & wxMINIMIZE_BOX))
		m_iButtonState[BUTTON_MIN] = BSTATE_DISABLED;

	m_imgBorder = GetGCThemeManager()->getImageHandle(imgActive);
	m_imgBorderNA  = GetGCThemeManager()->getImageHandle(imgNonActive);
}

void CustomFrameButtons::setActive(bool state)
{
	m_bActive = state;
	invalidateBitmap();
}

bool CustomFrameButtons::isActive()
{
	return m_bActive;
}

void CustomFrameButtons::invalidateBitmap()
{
	m_bButtonBitMapValid = false;
	m_pParent->RefreshRect(getRect(), false);
}

wxRect CustomFrameButtons::getRect()
{
	return wxRect(m_ButtonPos, m_ButtonBitMap.GetSize());
}

void CustomFrameButtons::clearBg(wxDC* dc)
{
	int ih = m_imgBorder->GetSize().GetHeight();
	int iw = m_imgBorder->GetSize().GetWidth();

	wxImage borderImg = isActive()?m_imgBorder->Scale(iw, ih):m_imgBorderNA->Scale(iw, ih);

	wxBitmap borderBMP = GetGCThemeManager()->getSprite(borderImg, "main_border", "fill");
	wxBitmap borderBMPTarg(m_ButtonBitMap.GetSize());

	gcImage::tileImg(borderBMPTarg, borderBMP, 0, gcImage::BOTH);
	dc->DrawBitmap(borderBMPTarg, wxPoint(0,0), true);
}

void CustomFrameButtons::generateButtons(wxDC* dc)
{
	if (!m_bButtonBitMapValid)
	{
		wxMemoryDC tmpDC(m_ButtonBitMap);
		clearBg(&tmpDC);

		for (size_t x=BUTTON_CLOSE; x<=BUTTON_MIN; x++)
		{
			int butId = x;

			if (butId == BUTTON_MAX && m_pFrame->IsMaximized() && !(m_iButtonState[BUTTON_MAX] == BSTATE_DISABLED))
				butId = BUTTON_RESTORE;

			gcString imgName = g_szImageNames[butId];

			if (!isActive())
				imgName += "_nonactive";

			gcImageHandle imgBut = GetGCThemeManager()->getImageHandle(imgName.c_str());
			wxImage buttonImage(imgBut->Copy());

			wxBitmap temp = GetGCThemeManager()->getSprite(buttonImage, "main_button", g_szSpriteNames[m_iButtonState[x]]);

			if (temp.IsOk())
				tmpDC.DrawBitmap(temp, m_ButtonRect[x].GetTopLeft(), true);
		}

		tmpDC.SelectObject(wxNullBitmap);
		m_bButtonBitMapValid=true;
	}

	dc->DrawBitmap(m_ButtonBitMap, m_ButtonPos, true);
}


int32 CustomFrameButtons::isWithingButtons(uint8 action)
{
	wxPoint wpos = m_pParent->GetScreenPosition();
	wxPoint mpos = wxGetMousePosition();
	wxPoint mousePos = mpos-wpos;

	int butId = -1;
	bool valid = true;

	for (size_t x=BUTTON_CLOSE; x<=BUTTON_MIN; x++)
	{
		wxRect rect = m_ButtonRect[x];
		rect.SetPosition(rect.GetPosition() + m_ButtonPos);

		if (rect.Contains(mousePos))
		{
			butId = x;
			break;
		}
	}

	if (butId != -1)
	{
		int state = m_iButtonState[butId];

		if (state != BSTATE_DISABLED)
		{
			int targetState;

			switch (action)
			{
				case MOUSE_LDOWN:	
					targetState = BSTATE_FOCUSE; 
					break;

				case MOUSE_MOVE:
					if (m_iMouseDown)
					{
						if (m_iMouseDown == butId+1)
							targetState = BSTATE_FOCUSE;
						else
							targetState = BSTATE_NORMAL; 
						break;
					}

				case MOUSE_LUP:		
				default:
					targetState = BSTATE_HOVER; 
					break;
			}

			if (state != targetState)
			{
				m_iButtonState[butId] = targetState;
				valid = false;
			}
		}
	}
		
	for (size_t x=BUTTON_CLOSE; x<=BUTTON_MIN; x++)
	{
#ifdef WIN32
		if (x == butId)
#else
		if (x == (uint64)butId)
#endif
			continue;

		if (m_iButtonState[x] != BSTATE_NORMAL && m_iButtonState[x] != BSTATE_DISABLED)
		{
			m_iButtonState[x] = BSTATE_NORMAL;
			valid = false;
		}
	}

	if (!valid)
		invalidateBitmap();

	return butId+1;
}


void CustomFrameButtons::calcBorders(int wb, int yoff)
{
	int butw = 0;

	for (size_t x=BUTTON_CLOSE; x<=BUTTON_MIN; x++)
		butw += m_ButtonRect[x].GetWidth();

	m_ButtonPos = wxPoint(wb-butw, yoff+1);
}

bool CustomFrameButtons::onMouseMove()
{
	if (isWithingButtons(MOUSE_MOVE))
	{
		if (!m_pParent->HasCapture())
			m_pParent->CaptureMouse();

		return true;
	}
	else
	{
		if (m_pParent->HasCapture())
			m_pParent->ReleaseMouse();
	}

	return false;
}

bool CustomFrameButtons::onMouseDown()
{
	int mouse = isWithingButtons(MOUSE_LDOWN);

	if (mouse)
		m_iMouseDown = mouse;

	return mouse?true:false;
}

bool CustomFrameButtons::onMouseUp()
{
	int oldmouse = m_iMouseDown;
	int mouse = isWithingButtons(MOUSE_LUP);

	m_iMouseDown = 0;

	if (mouse)
	{
		if (oldmouse != mouse)
			return false;

		if (mouse-1 == BUTTON_CLOSE && m_iButtonState[BUTTON_CLOSE] != BSTATE_DISABLED)
		{
			m_pFrame->Close();
		}
		else if (mouse-1 == BUTTON_MAX && m_iButtonState[BUTTON_MAX] != BSTATE_DISABLED)
		{
			bool isMaxed = m_pFrame->IsMaximized();
			m_pFrame->Maximize(!isMaxed);

			m_bButtonBitMapValid = false;
			m_iButtonState[BUTTON_MAX] = BSTATE_NORMAL;
		}
		else if (mouse-1 == BUTTON_MIN && m_iButtonState[BUTTON_MIN] != BSTATE_DISABLED)
		{
#ifdef WIN32
			::SendMessageA((HWND)m_pFrame->GetHWND(), WM_SYSCOMMAND, SC_MINIMIZE, 0);
#endif
		}

		return true;
	}

	return false;
}

bool CustomFrameButtons::onMouseDoubleClick()
{
	if (isWithingButtons(MOUSE_MOVE))
	{
		m_bButtonBitMapValid = false;
		return true;
	}

	return false;
}
