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
#include "gcCustomFrame.h"
#include "gcCustomFrameImpl.h"

const char* g_szBorderNames[] =
{
	"north",
	"neast",
	"east",
	"seast",
	"south",
	"swest",
	"west",
	"nwest",
};

enum
{
	SC_CUSTOM_MIN = 90001,
	SC_CUSTOM_MAX,
	SC_CUSTOM_RESTORE,
};

#define BORDER_SIZE 6
#define GetHrgnOf(rgn)          ((HRGN)(rgn).GetHRGN())

gcCustomFrameImpl::gcCustomFrameImpl(wxTopLevelWindow* parent, FrameIcon* frameIcon, long style)
{
	m_pCFButtons = new CustomFrameButtons(parent, parent, style, "#main_border", "#main_border_nonactive");
	m_pCFResize = new CustomFrameResize(parent, (style & wxRESIZE_BORDER)?true:false);
	m_pCFMove = new CustomFrameMove(parent);

	m_FrameBorder.resize(8);

	m_bDisableSetRegion = false;
	m_bActive = false;
	m_bBGBitMapValid= false;
	m_bShowTitle = true;
	m_bIsMaximized = false;
	m_bWasLastMaximised = false;

#ifdef WIN32
	m_iTitleHeight = 22;
#else
	m_iTitleHeight = 0;
#endif

	m_pParent = parent;
	m_pFrameIcon = frameIcon;

	m_bMaxEnabled = (style & wxMAXIMIZE_BOX)?true:false;
	m_bMinEnabled = (style & wxMINIMIZE_BOX)?true:false;

	m_LastSize = m_pParent->GetSize();

	calcBorders();

	m_imgBorder = GetGCThemeManager()->getImageHandle("#main_border");
	m_imgBorderNA = GetGCThemeManager()->getImageHandle("#main_border_nonactive");

	m_pParent->Bind(wxEVT_LEFT_DCLICK, &gcCustomFrameImpl::onLeftDClick, this);
	m_pParent->Bind(wxEVT_MOTION, &gcCustomFrameImpl::onMouseMove, this);
	m_pParent->Bind(wxEVT_LEFT_DOWN, &gcCustomFrameImpl::onLeftMouseDown, this);
	m_pParent->Bind(wxEVT_LEFT_UP, &gcCustomFrameImpl::onLeftMouseUp, this);
	m_pParent->Bind(wxEVT_RIGHT_DOWN, &gcCustomFrameImpl::onRightMouseDown, this);
	m_pParent->Bind(wxEVT_SIZE, &gcCustomFrameImpl::onResize, this);

	//m_pParent->Bind(wxEVT_PAINT, &gcCustomFrameImpl::onPaint, this);
	m_pParent->Bind(wxEVT_NC_PAINT, &gcCustomFrameImpl::onPaintNC, this);
	m_pParent->Bind(wxEVT_ERASE_BACKGROUND, &gcCustomFrameImpl::onPaintBG, this);

	m_pParent->Bind(wxEVT_KILL_FOCUS, &gcCustomFrameImpl::onBlur, this);
	m_pParent->Bind(wxEVT_MOUSE_CAPTURE_LOST, &gcCustomFrameImpl::onMouseCaptureLost, this);


	setFrameRegion();
	m_pParent->Layout();
	m_pParent->Refresh();

#ifdef WIN32
	HMENU menu = GetSystemMenu((HWND)m_pParent->GetHWND(), FALSE);

	RemoveMenu(menu, SC_RESTORE, MF_BYCOMMAND);
	RemoveMenu(menu, SC_MAXIMIZE, MF_BYCOMMAND);
	RemoveMenu(menu, SC_MINIMIZE, MF_BYCOMMAND);
	RemoveMenu(menu, SC_MOVE, MF_BYCOMMAND);
	RemoveMenu(menu, SC_SIZE, MF_BYCOMMAND);

	InsertMenu(menu, 0, MF_BYPOSITION, SC_CUSTOM_RESTORE, L"Restore");
	InsertMenu(menu, 1, MF_BYPOSITION, SC_CUSTOM_MIN, L"Minimize");
	InsertMenu(menu, 2, MF_BYPOSITION, SC_CUSTOM_MAX, L"Maximize");


	MENUITEMINFO mif;

	memset(&mif, 0, sizeof(MENUITEMINFO));
	mif.cbSize = sizeof(MENUITEMINFO);

	mif.fMask = MIIM_BITMAP;


	mif.hbmpItem = HBMMENU_POPUP_RESTORE;
	SetMenuItemInfo(menu, SC_CUSTOM_RESTORE, FALSE, &mif);

	mif.hbmpItem = HBMMENU_POPUP_MINIMIZE;
	SetMenuItemInfo(menu, SC_CUSTOM_MIN, FALSE, &mif);

	mif.hbmpItem = HBMMENU_POPUP_MAXIMIZE;
	SetMenuItemInfo(menu, SC_CUSTOM_MAX, FALSE, &mif);

	EnableMenuItem(menu, SC_CUSTOM_RESTORE, MF_GRAYED);

	if (!m_bMaxEnabled)
		EnableMenuItem(menu, SC_CUSTOM_MAX, MF_GRAYED);

	if (!m_bMinEnabled)
		EnableMenuItem(menu, SC_CUSTOM_MIN, MF_GRAYED);
#endif
}

gcCustomFrameImpl::~gcCustomFrameImpl()
{
	// Unbind Events
	m_pParent->Unbind( wxEVT_LEFT_DCLICK, &gcCustomFrameImpl::onLeftDClick, this);
	m_pParent->Unbind( wxEVT_MOTION, &gcCustomFrameImpl::onMouseMove, this);
	m_pParent->Unbind( wxEVT_LEFT_DOWN, &gcCustomFrameImpl::onLeftMouseDown, this);
	m_pParent->Unbind( wxEVT_LEFT_UP, &gcCustomFrameImpl::onLeftMouseUp, this);
	m_pParent->Unbind( wxEVT_PAINT, &gcCustomFrameImpl::onPaint, this);
	m_pParent->Unbind( wxEVT_SIZE, &gcCustomFrameImpl::onResize, this);
	m_pParent->Unbind( wxEVT_ERASE_BACKGROUND, &gcCustomFrameImpl::onPaintBG, this);

	m_pParent->Unbind( wxEVT_KILL_FOCUS, &gcCustomFrameImpl::onBlur, this);
	m_pParent->Unbind( wxEVT_MOUSE_CAPTURE_LOST, &gcCustomFrameImpl::onMouseCaptureLost, this);

	if (m_pParent->HasCapture())
		m_pParent->ReleaseMouse();

	safe_delete(m_pCFButtons);
	safe_delete(m_pCFMove);
	safe_delete(m_pCFResize);
}

void gcCustomFrameImpl::destroy()
{
}

bool gcCustomFrameImpl::isResizing()
{
	return m_pCFResize->isResizing();
}

#ifdef WIN32
WXLRESULT gcCustomFrameImpl::PreMSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam, bool &processed)
{
	if (message == WM_NCACTIVATE)
	{
		m_bActive = wParam?true:false;

		if (m_pCFButtons)
			m_pCFButtons->setActive(m_bActive);

		m_pParent->Refresh(false);

		bool state = m_bActive;
		onActiveEvent(state);
	}
	else if (message == WM_SYSCOMMAND || message == WM_COMMAND)
	{
		if (wParam == SC_MINIMIZE || wParam == SC_MAXIMIZE || wParam == SC_CUSTOM_MIN || wParam == SC_CUSTOM_MAX)
		{
			m_bDisableSetRegion = true;
		}
		else if (wParam == SC_RESTORE || wParam == SC_CUSTOM_RESTORE)
		{
			m_bDisableSetRegion = false;
		}

		if (wParam == SC_MAXIMIZE || wParam == SC_CUSTOM_MAX)
		{
			m_pParent->Maximize(true);
			processed = true;
		}
		else if (wParam == SC_RESTORE || wParam == SC_CUSTOM_RESTORE)
		{
			if (m_bWasLastMaximised)
			{
				m_bWasLastMaximised = false;
				m_pParent->Restore();
				m_pParent->SetSize(m_RestorSize);
				m_pParent->Maximize(true);
			}
			else if (m_pParent->IsMaximized())
			{
				m_pParent->Maximize(false);
			}
			else
			{
				m_pParent->Restore();
			}

			processed = true;
		}
		else if (wParam == SC_MINIMIZE || wParam == SC_CUSTOM_MIN)
		{
			m_pParent->Iconize();
			processed = true;
		}
		else if (wParam == SC_CLOSE)
		{
			m_pParent->Close();
			processed = true;
		}
	}
	else if (message == WM_WINDOWPOSCHANGING)
	{
		WINDOWPOS* pos = (WINDOWPOS*)lParam;

		if (pos && (pos->flags & SWP_NOSIZE) == 0)
		{
			calcBorders();
			wxSize newSize(pos->cx, pos->cy);
			
			//issues when maxing and mining if we setframe during them
			if (newSize != wxSize(0,0) && newSize != m_LastSize && !m_bDisableSetRegion)
			{
				setFrameRegion();
				m_LastSize = newSize;
			}
		}
	}
	else if (message == WM_SIZE)
	{
		if (wParam == SIZE_MINIMIZED && m_bIsMaximized)
		{
			m_bWasLastMaximised = true;
			m_bIsMaximized = false;
			m_pParent->SetSize(m_LastSize);
		}
		else if (wParam == SIZE_MAXIMIZED)
		{
			m_bIsMaximized = true;
		}
	}

	return 0;
}

WXLRESULT gcCustomFrameImpl::PostMSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam, WXLRESULT res)
{
	if (message == WM_SIZE && wParam == SIZE_RESTORED)
	{
		m_pParent->Layout();
	}

	return res;
}
#endif

void gcCustomFrameImpl::setTitleHeight(unsigned int height)
{
	if (height<1)
		height = 1;

	m_iTitleHeight = height;
	m_pParent->Refresh();
}

void gcCustomFrameImpl::showTitle(bool state)
{
	m_bShowTitle = state;
	m_pParent->Refresh();
}

//Some reason wxWidgets draws a 1px border around the top and left sides. Increase client area by one and reduce th client area offset


wxRect gcCustomFrameImpl::getWindowsBorderRect() const
{
	// SetWindowRgn expects the region to be in coordinants
	// relative to the window, not the client area.  Figure
	// out the offset, if any.
#ifdef WIN32
	RECT rect;
	DWORD dwStyle =   ::GetWindowLong((HWND)m_pParent->GetHWND(), GWL_STYLE);
	DWORD dwExStyle = ::GetWindowLong((HWND)m_pParent->GetHWND(), GWL_EXSTYLE);
	::GetClientRect((HWND)m_pParent->GetHWND(), &rect);
	::AdjustWindowRectEx(&rect, dwStyle, ::GetMenu((HWND)m_pParent->GetHWND()) != NULL, dwExStyle);

	int x=rect.left*-1;
	int y=rect.top*-1;

	int w = rect.right+rect.left;
	int h = rect.bottom+rect.left;//+ (y-x)-1;

	wxPoint point = m_pParent->GetPosition();

	return wxRect(wxPoint(x, y), wxSize(w, h));
#else
	return wxRect(wxPoint(0,0), wxSize(800, 600));
#endif
}


void gcCustomFrameImpl::DoGetClientSize(int *width, int *height) const
{
	*width = m_ClientArea.GetWidth();
	*height = m_ClientArea.GetHeight();
}

wxPoint gcCustomFrameImpl::GetClientAreaOrigin() const
{
	return m_PaintClientArea.GetTopLeft();
}

void gcCustomFrameImpl::restore()
{
#ifdef WIN32
	HMENU menu = GetSystemMenu((HWND)m_pParent->GetHWND(), FALSE);

	EnableMenuItem(menu, SC_CUSTOM_RESTORE, MF_GRAYED);

	if (m_bMaxEnabled)
		EnableMenuItem(menu, SC_CUSTOM_MAX, MF_ENABLED);

	if (m_bMinEnabled)
		EnableMenuItem(menu, SC_CUSTOM_MIN, MF_ENABLED);
#endif
}

void gcCustomFrameImpl::maximize(bool state)
{
#ifdef WIN32
	if (!m_bMaxEnabled)
		return;

	if (state == m_bIsMaximized)
		return;

	if (m_pParent->IsIconized())
		m_pParent->Iconize(false);

	if (m_bIsMaximized)
	{
		doRestore();
	}
	else
	{
		doMaximize();
	}

	HMENU menu = GetSystemMenu((HWND)m_pParent->GetHWND(), FALSE);

	if (state)
	{
		EnableMenuItem(menu, SC_CUSTOM_RESTORE, MF_ENABLED);
		EnableMenuItem(menu, SC_CUSTOM_MAX, MF_GRAYED);
	}
	else
	{
		EnableMenuItem(menu, SC_CUSTOM_RESTORE, MF_GRAYED);
		EnableMenuItem(menu, SC_CUSTOM_MAX, MF_ENABLED);
	}

	m_bIsMaximized = state;

	m_pParent->Layout();
#endif
}

void gcCustomFrameImpl::minimize(bool state)
{
#ifdef WIN32
	if (!m_bMinEnabled)
		return;

	HMENU menu = GetSystemMenu((HWND)m_pParent->GetHWND(), FALSE);

	if (state)
	{
		EnableMenuItem(menu, SC_CUSTOM_RESTORE, MF_ENABLED);
		EnableMenuItem(menu, SC_CUSTOM_MIN, MF_GRAYED);
	}
	else
	{
		EnableMenuItem(menu, SC_CUSTOM_RESTORE, MF_GRAYED);
		EnableMenuItem(menu, SC_CUSTOM_MIN, MF_ENABLED);
	}
#endif
}

void gcCustomFrameImpl::doRestore()
{
	if (!m_bIsMaximized)
		return;

	m_bIsMaximized = false;

	m_pParent->SetSize(m_RestorSize);
	m_pParent->Layout();

	m_RestorSize = wxRect();
}

void gcCustomFrameImpl::doMaximize()
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

	m_pParent->SetSize(wxRect(pos, size));
#endif 
	if (m_pCFButtons)
		m_pCFButtons->invalidateBitmap();

	calcBorders();
	setFrameRegion();
	m_pParent->Refresh(true);
}

void gcCustomFrameImpl::onResize( wxSizeEvent& event )
{
	m_bBGBitMapValid = false;

	setFrameRegion();

	m_pParent->Layout();
	m_pParent->Refresh();

	event.Skip();
}


wxRegion gcCustomFrameImpl::generateBorderRegion()
{
	wxRect butRect;
	
	if (m_pCFButtons)
		butRect = m_pCFButtons->getRect();

	wxRegion bRegion;
	
	for (int x=0; x<BORDER_DIR_MAX; x++)
		bRegion.Union(m_FrameBorder[x]);

	bRegion.Subtract(butRect);
	bRegion.Subtract(m_TitleBorder);
	bRegion.Subtract(m_ClientArea);

	return bRegion;
}

void gcCustomFrameImpl::doPaintBorder(wxDC* dc)
{
	calcBorders();

	wxRect butRect;
	
	if (m_pCFButtons)
		butRect = m_pCFButtons->getRect();

	dc->DestroyClippingRegion();
	dc->SetDeviceClippingRegion(generateBorderRegion());

	generateBorder(dc);

	wxRegion tRegion(m_TitleBorder);
	tRegion.Subtract(butRect);

	dc->DestroyClippingRegion();
	dc->SetDeviceClippingRegion(tRegion);

	generateTitle(dc);

	dc->DestroyClippingRegion();

	if (m_pCFButtons)
	{
		dc->SetDeviceClippingRegion(wxRegion(butRect));
		m_pCFButtons->generateButtons(dc);
		dc->DestroyClippingRegion();
	}

	m_bBGBitMapValid = true;
}

void gcCustomFrameImpl::onPaintNC(wxNcPaintEvent& event)
{
	wxWindowDC dc(m_pParent);
	
	wxRect pRect = m_pParent->GetRect();
	wxSize pSize = pRect.GetSize();

	if (!m_BorderBitmap.IsOk() || m_BorderBitmap.GetSize() != pSize)
		m_BorderBitmap = wxBitmap(pSize);

	wxMemoryDC tmpDC(m_BorderBitmap);
	doPaintBorder(&tmpDC);
	tmpDC.SelectObject(wxNullBitmap);

	wxRegion region(wxRect(wxPoint(0,0), pSize));
	region.Subtract(m_ClientArea);

	dc.DestroyClippingRegion();
	dc.SetDeviceClippingRegion(region);
	dc.DrawBitmap(m_BorderBitmap, wxPoint(0,0), true);
}

void gcCustomFrameImpl::onPaintBG( wxEraseEvent& event )
{
	wxDC* dc = event.GetDC();

	dc->DestroyClippingRegion();
	dc->SetDeviceClippingRegion(m_PaintClientArea);

	dc->SetPen(wxPen(m_pParent->GetBackgroundColour()));
	dc->SetBrush(wxBrush(m_pParent->GetBackgroundColour()));
	dc->DrawRectangle(m_PaintClientArea);

	dc->DestroyClippingRegion();

	wxNcPaintEvent ncE;
	onPaintNC(ncE);
}

void gcCustomFrameImpl::onPaint(wxPaintEvent& event)
{
	//wxPaintDC dc(m_pParent);

	//if (m_pParent->IsMaximized())
	//{
	//	dc.SetPen(wxPen(*wxCYAN));
	//	dc.SetBrush(wxBrush(*wxCYAN));
	//}
	//else
	//{
	//	dc.SetPen(wxPen(*wxGREEN));
	//	dc.SetBrush(wxBrush(*wxGREEN));
	//}

	//dc.DrawRectangle(wxPoint(0,0), m_pParent->GetSize());
}




void gcCustomFrameImpl::generateBorder(wxDC* dc)
{
	if (m_pParent->IsMaximized())
		return;

	int ih = m_imgBorder->GetSize().GetHeight();
	int iw = m_imgBorder->GetSize().GetWidth();

	wxImage borderImg = m_bActive?m_imgBorder->Scale(iw, ih):m_imgBorderNA->Scale(iw, ih);

	//dc->SetBrush(wxBrush(*wxRED));
	//dc->SetPen(wxPen(*wxRED));

	for (int x=0; x<BORDER_DIR_MAX; x++)
	{
		wxBitmap borderBMP = GetGCThemeManager()->getSprite(borderImg, "main_border", g_szBorderNames[x]);
		wxBitmap borderBMPTarg(m_FrameBorder[x].GetSize());

		if (!borderBMP.IsOk())
			continue;

		if (x == NORTH || x == SOUTH)
		{
			wxColor c(255,0,255);
			gcImage::tileImg(borderBMPTarg, borderBMP, &c);
		}
		else if (x == WEST || x == EAST)
		{
			wxColor c(255,0,255);
			gcImage::tileImg(borderBMPTarg, borderBMP, &c, gcImage::VERT);
		}
		else
		{
			borderBMPTarg = wxBitmap(borderBMP.ConvertToImage().Scale(m_FrameBorder[x].GetWidth(), m_FrameBorder[x].GetHeight()));
		}

		if (borderBMPTarg.IsOk())
			dc->DrawBitmap(borderBMPTarg, m_FrameBorder[x].GetPosition(), true);
	}
}

void gcCustomFrameImpl::generateTitle(wxDC *dc)
{
	if (!m_bShowTitle)
		return;

	//dc->SetBrush(wxBrush(*wxBLUE));
	//dc->SetPen(wxPen(*wxBLUE));
	//dc->DrawRectangle(m_TitleBorder);

	int ih = m_imgBorder->GetSize().GetHeight();
	int iw = m_imgBorder->GetSize().GetWidth();

	wxImage borderImg = m_bActive?m_imgBorder->Scale(iw, ih):m_imgBorderNA->Scale(iw, ih);
	wxBitmap borderBMP = GetGCThemeManager()->getSprite(borderImg, "main_border", "fill");
	wxBitmap borderBMPTarg(m_TitleBorder.GetSize());

	gcImage::tileImg(borderBMPTarg, borderBMP, 0, gcImage::BOTH);

	if (borderBMPTarg.IsOk())
		dc->DrawBitmap(borderBMPTarg, m_TitleBorder.GetPosition(), true);

	if (m_pFrameIcon)
	{
		wxIcon icon = m_pFrameIcon->getFrameIcon();

		if (icon.IsOk())
			dc->DrawIcon(icon, m_IconRect.GetTopLeft().x, m_IconRect.GetTopLeft().y);
	}

	wxString title = m_pParent->GetTitle();

	if (title.size() > 0)
	{
		wxFont font = dc->GetFont();
		font.SetWeight(wxFONTWEIGHT_LIGHT);

		dc->SetFont(font);
		dc->SetTextForeground(m_pParent->GetForegroundColour());

		wxSize tsize = dc->GetTextExtent(title);

		int tx = m_IconRect.GetTopLeft().x+m_IconRect.GetSize().x+5;
		int ty = m_IconRect.GetTopLeft().y + (m_IconRect.GetSize().y-tsize.GetHeight())/2;

		dc->DrawText(title, tx, ty);
	}
}

void gcCustomFrameImpl::setFrameRegion()
{
	wxRect wbr = getWindowsBorderRect();
	wxPoint br = wbr.GetBottomRight();
	
	br.x += 2;
	br.y += 2;

	wxRegion region;

#ifdef WIN32
	if (m_pParent->IsMaximized())
	{
		region = CreateRectRgn(wbr.x, wbr.y, br.x, br.y);
	}
	else
	{
		region = CreateRoundRectRgn(wbr.x, wbr.y, br.x, br.y, 5, 5);
	}

	// Windows takes ownership of the region, so
	// we'll have to make a copy of the region to give to it.

	DWORD noBytes = ::GetRegionData(GetHrgnOf(region), 0, NULL);
	RGNDATA *rgnData = (RGNDATA*) new char[noBytes];
	::GetRegionData(GetHrgnOf(region), noBytes, rgnData);
	HRGN hrgn = ::ExtCreateRegion(NULL, noBytes, rgnData);
	delete[] (char*) rgnData;

	// Now call the shape API with the new region.
	::SetWindowRgn((HWND)m_pParent->GetHWND(), hrgn, FALSE);
#endif
}

void gcCustomFrameImpl::onLeftDClick( wxMouseEvent& event )
{
	wxPoint pos = wxGetMousePosition()-m_pParent->GetPosition();

	if (m_pCFButtons && m_pCFButtons->onMouseDoubleClick())
	{
		event.Skip();
		return; 
	}

	if (m_bMaxEnabled && m_TitleBorder.Contains(pos))
	{
		bool isMaxed = m_pParent->IsMaximized();
		m_pParent->Maximize(!isMaxed);
	}
	else
	{
		event.Skip();
	}
}

void gcCustomFrameImpl::onMouseCaptureLost(wxMouseCaptureLostEvent &event)
{
	if (m_pParent->HasCapture())
		m_pParent->ReleaseMouse();

	wxMouseEvent mouseEvent;

	onLeftMouseUp(mouseEvent);

}

void gcCustomFrameImpl::onBlur(wxFocusEvent& event)
{
	if (m_pParent->HasCapture())
		m_pParent->ReleaseMouse();

	event.Skip();
}


void gcCustomFrameImpl::onMouseMove( wxMouseEvent& event )
{
	if (m_pCFMove->isDragging())
	{
		if (m_pCFMove->onMouseMove())
		{
			m_pCFResize->reset();
		}
	}
	else if (m_pCFResize->isResizing())
	{
		m_pCFResize->onMouseMove();
	}
	else 
	{
		if (m_pCFButtons && m_pCFButtons->onMouseMove())
		{
			m_pCFResize->reset();
		}
		else if (m_pCFMove->onMouseMove())
		{
			m_pCFResize->reset();

			if (m_pCFButtons)
				m_pCFButtons->invalidateBitmap();
		}
		else if (m_pCFResize->onMouseMove())
		{
		}
	}

	event.Skip();
}

void gcCustomFrameImpl::onRightMouseDown(wxMouseEvent& event)
{
	wxPoint pos = wxGetMousePosition()-m_pParent->GetPosition();

	if (m_TitleBorder.Contains(pos))
	{
#ifdef WIN32
		wxPoint mousePos = wxGetMousePosition();
		HMENU menu = GetSystemMenu((HWND)m_pParent->GetHWND(), FALSE);
		TrackPopupMenu(menu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, mousePos.x, mousePos.y, 0, (HWND)m_pParent->GetHWND(), NULL);
#endif
	}
	else
	{
		event.Skip();
	}
}

void gcCustomFrameImpl::onLeftMouseDown( wxMouseEvent& event )
{
	if ((m_pCFButtons && m_pCFButtons->onMouseDown()) || m_pCFMove->onMouseDown(m_pCFResize->canResize()) || m_pCFResize->onMouseDown())
		event.Skip();
}

void gcCustomFrameImpl::onLeftMouseUp( wxMouseEvent& event )
{
	if ((m_pCFButtons && m_pCFButtons->onMouseUp()) || m_pCFMove->onMouseUp() || m_pCFResize->onMouseUp())
		event.Skip();
}

void gcCustomFrameImpl::calcBorders()
{
	wxRect wbr = getWindowsBorderRect();

	int borderSize = BORDER_SIZE;
	int titleHeight = m_iTitleHeight + borderSize;

	if (m_pParent->IsMaximized())
		borderSize = 0;

	int x = wbr.x;
	int y = wbr.y;

	int w = wbr.width;
	int h = wbr.height;

	int wb = w - borderSize;
	int hb = h - borderSize;
	int w2b = w - borderSize*2;
	int h2b = h - borderSize*2;

	m_PaintClientArea =		wxRect(borderSize,		titleHeight,	w2b+x,		h-titleHeight-borderSize+y);
	m_ClientArea =			wxRect(x+borderSize,	y+titleHeight,	w2b,		h-titleHeight-borderSize);
	m_TitleAreaRect =		wxRect(x+0,				y+0,			w,			titleHeight);
	m_TitleBorder =			wxRect(x+borderSize,	y+borderSize,	w2b,		titleHeight-borderSize);

	m_FrameBorder[NORTH] =	wxRect(x+borderSize,	y+0,			w2b,		borderSize);
	m_FrameBorder[NEAST] =	wxRect(x+wb,			y+0,			borderSize, borderSize);
	m_FrameBorder[EAST] =	wxRect(x+wb,			y+borderSize,	borderSize, h2b);
	m_FrameBorder[SEAST] =	wxRect(x+wb,			y+hb,			borderSize, borderSize);

	m_FrameBorder[SOUTH] =	wxRect(x+borderSize,	y+hb,			w2b,		borderSize);
	m_FrameBorder[SWEST] =	wxRect(x+0,				y+hb,			borderSize, borderSize);
	m_FrameBorder[WEST] =	wxRect(x+0,				y+borderSize,	borderSize, h2b);
	m_FrameBorder[NWEST] =	wxRect(x+0,				y+0,			borderSize, borderSize);

	int iSize = 16;
	int iIndenet = (titleHeight-iSize)/2;
	m_IconRect = wxRect(x+borderSize, y+iIndenet, iSize, iSize);

	if (m_pCFButtons)
		m_pCFButtons->calcBorders(wb+x, y);

	m_pCFResize->calcBorders(m_FrameBorder);
	m_pCFMove->calcBorders(m_TitleBorder, m_TitleAreaRect);
}
