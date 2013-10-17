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

#include <stdio.h>
#include "DesuraProgress.h"
#include "resource.h"

#define MIN( a, b) (a)>(b)?(b):(a)
#define MAX( a, b) (a)>(b)?(a):(b)

#define DPROGRESS_CLASSNAME "MFCDesuraProgress"  // Window class name

// these macros are standard now (Win98) but some older headers don't have them
#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
#define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif // GET_X_LPARAM

using namespace Desurium;

DesuraProgress* DesuraProgress::gs_pDesuraProgress = NULL;

DesuraProgress::DesuraProgress()
	: CDesuraWnd(true)
{
	gs_pDesuraProgress = this;

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
	gs_pDesuraProgress = NULL;

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


bool DesuraProgress::RegisterWindowClass()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = GetInstanceHandle();

	if (!(::GetClassInfo(hInst, DPROGRESS_CLASSNAME, &wndcls)))
	{
		// otherwise we need to register a new class

		wndcls.style            = CS_HREDRAW | CS_VREDRAW|CS_DBLCLKS;
		wndcls.lpfnWndProc      = &DesuraProgress::WinProc;
		wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
		wndcls.hInstance        = hInst;
		wndcls.hIcon            = NULL;
		wndcls.hCursor          = LoadStandardCursor(IDC_ARROW);
		wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
		wndcls.lpszMenuName     = NULL;
		wndcls.lpszClassName    = DPROGRESS_CLASSNAME;

		if (!RegisterClass(&wndcls))
			return false;
	}

	return true;
}

LRESULT DesuraProgress::WinProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		gs_pDesuraProgress->OnPaint();
		return 0;

	case WM_ERASEBKGND:
		gs_pDesuraProgress->OnEraseBkgnd();
		return 1;

	case WM_MOUSEMOVE:
		gs_pDesuraProgress->OnMouseMove(wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
		return 0;

	case WM_MOUSELEAVE:
		gs_pDesuraProgress->OnMouseLeave();
		return 0;

	case WM_LBUTTONDOWN:
		gs_pDesuraProgress->OnLButtonDown(wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
		return 0;

	case WM_LBUTTONUP:
		gs_pDesuraProgress->OnLButtonUp(wParam, CPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
		return 0;
	}

	return ::DefWindowProc(hDlg, message, wParam, lParam);
}

bool DesuraProgress::Create(CDesuraWnd* pParentWnd, const CRect& rect, UINT nID, DWORD dwStyle /*=WS_VISIBLE*/)
{
	return __super::Create(DPROGRESS_CLASSNAME, "Abcd", dwStyle, rect, pParentWnd, nID);
}

void DesuraProgress::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_bDragging = false;
	ReleaseCapture();
}

void DesuraProgress::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rect = getCancelRect();
	if (rect.PtInRect(point))
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
	m_bMouseOver = rect.PtInRect(point);

	if (old != m_bMouseOver)
		refresh();

	if (m_bDragging)
	{
		CPoint pos = point - m_StartPoint;
		ClientToScreen(pos);
		GetParent()->SetWindowPos(pos.x-4, pos.y-26, 0, 0, SWP_NOSIZE);
	}
}


CRect DesuraProgress::getCancelRect()
{
	return CRect(280, 168, 100, 26);
}


void DesuraProgress::OnPaint() 
{
	CPaintDC dc(this);

	CRect size;
	GetClientRect(size);

	int w = size.right-size.left;
	int h = size.bottom-size.top;

	
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(dc, w, h);

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	HGDIOBJ pOldBitmap = memDC.SelectObject(&bmp);
	//memDC.FillSolidRect(0,0,w, h, RGB(34,34,34));
	
	{
		CDC cdc;
		BITMAP bmap;

		cdc.CreateCompatibleDC(&dc);
		HGDIOBJ pOldBitmap = cdc.SelectObject(m_pBackgroundImage);
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
		HGDIOBJ pOldBitmap = cdc.SelectObject(m_pLoadingBar);
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

bool DesuraProgress::OnEraseBkgnd() 
{
	return true;
}



