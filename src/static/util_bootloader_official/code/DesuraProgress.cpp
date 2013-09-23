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

// DesuraProgress.cpp : implementation file
//

#include "stdafx.h"
#include "DesuraProgress.h"
#include "resource.h"

#define MIN( a, b) (a)>(b)?(b):(a)
#define MAX( a, b) (a)>(b)?(a):(b)

#define DPROGRESS_CLASSNAME    _T("MFCDesuraProgress")  // Window class name














// DesuraProgress

IMPLEMENT_DYNAMIC(DesuraProgress, CWnd)

BEGIN_MESSAGE_MAP(DesuraProgress, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


DesuraProgress::DesuraProgress()
{
	m_iProg = 0;
	m_iTotal = 0;
	m_vDone[0] = 0;
	m_vDone[1] = 0;
	m_vDone[2] = 0;
	m_iMode = MODE_STARTUP;
	m_bMouseOver= false;

	m_pBackgroundImage = new CBitmap();
	m_pBackgroundImage->LoadBitmap(IDB_BITMAP1);

	m_pLoadingBar = new CBitmap();
	m_pLoadingBar->LoadBitmap(IDB_BITMAP2);

	m_bDragging = false;

	RegisterWindowClass();
}

DesuraProgress::~DesuraProgress()
{
	delete m_pBackgroundImage;
	m_pBackgroundImage = NULL;

	delete m_pLoadingBar;
	m_pLoadingBar = NULL;
}

void DesuraProgress::setDone(int done)
{
	m_vDone[0] = m_vDone[1];
	m_vDone[1] = m_vDone[2];
	m_vDone[2] = done;
}

int DesuraProgress::getDone()
{
	return (m_vDone[0]+m_vDone[1]+m_vDone[2])/3;
}


BOOL DesuraProgress::RegisterWindowClass()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	if (!(::GetClassInfo(hInst, DPROGRESS_CLASSNAME, &wndcls)))
	{
		// otherwise we need to register a new class

		wndcls.style            = CS_HREDRAW | CS_VREDRAW|CS_DBLCLKS;
		wndcls.lpfnWndProc      = ::DefWindowProc;
		wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
		wndcls.hInstance        = hInst;
		wndcls.hIcon            = NULL;
		wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
		wndcls.lpszMenuName     = NULL;
		wndcls.lpszClassName    = DPROGRESS_CLASSNAME;

		if (!AfxRegisterClass(&wndcls))
		{
			AfxThrowResourceException();
			return FALSE;
		}
	}

	return TRUE;
}


BOOL DesuraProgress::Create(CWnd* pParentWnd, const RECT& rect, UINT nID, DWORD dwStyle /*=WS_VISIBLE*/)
{
	return CWnd::Create(DPROGRESS_CLASSNAME, _T(""), dwStyle, rect, pParentWnd, nID);
}

void DesuraProgress::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bDragging = false;
	ReleaseCapture();
}

void DesuraProgress::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rect = getCancelRect();
	if (rect.PtInRect(point)==TRUE)
	{
		exit(1);
	}
	else
	{
		m_bDragging = true;
		m_StartPoint = point;
		SetCapture();
	}
}

void DesuraProgress::OnMouseLeave()
{
	bool old = m_bMouseOver;
	m_bMouseOver = false;
	if (old != m_bMouseOver)
		this->refresh();
}

void DesuraProgress::OnMouseMove(UINT nFlags, CPoint point) 
{
	bool old = m_bMouseOver;

	CRect rect = getCancelRect();
	m_bMouseOver = (rect.PtInRect(point)==TRUE);

	if (old != m_bMouseOver)
		refresh();

	if (m_bDragging)
	{
		CPoint pos = point - m_StartPoint;
		ClientToScreen(&pos);
		GetParent()->SetWindowPos(NULL, pos.x-4, pos.y-26, 0, 0, SWP_NOSIZE);
	}
}












CRect DesuraProgress::getCancelRect()
{
	SIZE s;
	s.cx = 100;
	s.cy = 26;

	POINT p;
	p.x = 280;
	p.y = 168;

	return CRect(p, s);
}


void DesuraProgress::OnPaint() 
{
	CPaintDC dc(this);

	RECT size;
	GetClientRect(&size);

	int w = size.right-size.left;
	int h = size.bottom-size.top;

	
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(&dc, w, h);

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap* pOldBitmap = memDC.SelectObject(&bmp);
	//memDC.FillSolidRect(0,0,w, h, RGB(34,34,34));
	
	{
		CDC cdc;
		BITMAP bmap;

		cdc.CreateCompatibleDC(&dc);
		CBitmap* pOldBitmap = cdc.SelectObject(m_pBackgroundImage);
		m_pBackgroundImage->GetBitmap(&bmap);

		memDC.StretchBlt(0, 0, w, h, &cdc, 0, 0, bmap.bmWidth, bmap.bmHeight, SRCCOPY);
		cdc.SelectObject(pOldBitmap);
	}


	COLORREF clr = RGB(103,103,103);
	int y1 = 108;
	int x1 = 38;

	//progress box
	{
		CDC cdc;
		BITMAP bmap;

		cdc.CreateCompatibleDC(&dc);
		CBitmap* pOldBitmap = cdc.SelectObject(m_pLoadingBar);
		m_pLoadingBar->GetBitmap(&bmap);

		int bw = bmap.bmWidth *m_iProg/100;


		memDC.BitBlt(x1, y1, bw, bmap.bmHeight, &cdc, bmap.bmWidth - bw, 0, SRCCOPY);
		cdc.SelectObject(pOldBitmap);
	}


	CFont Font2;
	Font2.CreatePointFont(80, "MS Shell Dlg");

	memDC.SelectObject(&Font2);
	memDC.SetBkMode(TRANSPARENT); 
	memDC.SetTextColor(clr);

	
	int tx = 430;
	int ty = 10;
	char progress[255] = {0};
	const char* process = NULL;

	if (m_iMode == MODE_STARTUP)
	{
		progress[0] = 0;
		process = "Initialising";
	}
	else
	{
		process = (m_iMode == MODE_DOWNLOADING)?"Downloading":"Installing";

		if (m_iTotal == 0)
			_snprintf_s(progress, 255, _TRUNCATE, "%0d%%", m_iProg);
		else
			_snprintf_s(progress, 255, _TRUNCATE, "%.2f/%.2f MiB", getDone()/1024.0/1024.0, m_iTotal/1024.0/1024.0);
	}

	CSize tsize = memDC.GetOutputTextExtent(progress, strlen(progress));
	CSize psize = memDC.GetOutputTextExtent(process, strlen(process));
	CSize csize = memDC.GetOutputTextExtent("Cancel", 6);

	tx = 148 + (107-tsize.cx)/2;
	ty = 171 + (21-csize.cy)/2;

	memDC.TextOut(tx, ty, progress, strlen(progress));


	tx = 32 + (106-psize.cx)/2;
	ty = 171 + (21-csize.cy)/2;

	memDC.TextOut(tx, ty, process, strlen(process));

	if (m_bMouseOver)
		memDC.SetTextColor(RGB(33,138,244));
	

	tx = 307 + (75-csize.cx)/2;
	ty = 171 + (21-csize.cy)/2;

	memDC.TextOut(tx, ty, "Cancel", 6);

	dc.BitBlt(0, 0, w, h, &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBitmap);
}

BOOL DesuraProgress::OnEraseBkgnd(CDC* pDC) 
{
	return true;
}



