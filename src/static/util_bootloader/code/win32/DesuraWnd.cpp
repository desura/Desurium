/*
Desura is the leading indie game distribution platform
Copyright (C) 2013 Mark Chandler (Desura Net Pty Ltd)

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

#include "DesuraWnd.h"


using namespace Desurium;



CBitmap::CBitmap()
: m_hBitmap(NULL)
{
}

CBitmap::~CBitmap()
{
	if (m_hBitmap)
		DeleteObject(m_hBitmap);
}

void CBitmap::LoadBitmap(int nResourceId)
{
	if (m_hBitmap)
		DeleteObject(m_hBitmap);

	m_hBitmap = ::LoadBitmap(CDesuraWnd::GetInstanceHandle(), MAKEINTRESOURCE(nResourceId));
}

void CBitmap::CreateCompatibleBitmap(CDC &dc, int w, int h)
{
	if (m_hBitmap)
		DeleteObject(m_hBitmap);

	m_hBitmap = ::CreateCompatibleBitmap(dc, w, h);
}

int CBitmap::GetBitmap(BITMAP *pBitmap)
{
	return GetObject(m_hBitmap, sizeof(BITMAP), pBitmap);
}






CDC::CDC()
	: m_hDC(NULL)
{
}

CDC::CDC(HDC hdc)
	: m_hDC(hdc)
{
}

CDC::~CDC()
{
	if (m_hDC)
		DeleteDC(m_hDC);
}

void CDC::SetBkMode(int nMode)
{
	assert(m_hDC);
	::SetBkMode(m_hDC, nMode);
}

void CDC::SetTextColor(COLORREF nColor)
{
	assert(m_hDC);
	::SetTextColor(m_hDC, nColor);
}

void CDC::CreateCompatibleDC(CDC* pDC)
{
	if (m_hDC)
		DeleteDC(m_hDC);

	m_hDC = ::CreateCompatibleDC(*pDC);

	int nError = GetLastError();
	assert(m_hDC);
}

HGDIOBJ CDC::SelectObject(CBitmap* pBitmap)
{
	assert(m_hDC);
	return ::SelectObject(m_hDC, *pBitmap);
}

HGDIOBJ CDC::SelectObject(HGDIOBJ pObject)
{
	assert(m_hDC);
	return ::SelectObject(m_hDC, pObject);
}

CFont* CDC::SelectObject(CFont* pFont)
{
	assert(m_hDC);
	return NULL;
}

void CDC::StretchBlt(int x, int y, int nWidth, int nHeight, CDC* pSrcDC,
	int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop)
{
	assert(m_hDC);
	::StretchBlt(m_hDC, x, y, nWidth, nHeight, *pSrcDC, xSrc, ySrc, nSrcWidth, nSrcHeight, dwRop);
}

void CDC::BitBlt(int x, int y, int nWidth, int nHeight, CDC* pSrcDC,
	int xSrc, int ySrc, DWORD dwRop)
{
	assert(m_hDC);
	::BitBlt(m_hDC, x, y, nWidth, nHeight, *pSrcDC, xSrc, ySrc, dwRop);
}

CSize CDC::GetOutputTextExtent(const char* szText, int nSize)
{
	assert(m_hDC);

	SIZE size;
	::GetTextExtentPoint32(m_hDC, szText, nSize, &size);

	return CSize(size);
}

bool CDC::TextOut(int x, int y, const char* szText, int nSize)
{
	assert(m_hDC);
	return ::TextOut(m_hDC, x, y, szText, nSize);
}



CPaintDC::CPaintDC(CDesuraWnd* pWnd)
	: CDC(::BeginPaint(pWnd->GetSafeHwnd(), &m_lpPaint))
	, m_pWnd(pWnd)
{

}

CPaintDC::~CPaintDC()
{
	::EndPaint(m_pWnd->GetSafeHwnd(), &m_lpPaint);
}




CDesuraWnd::CDesuraWnd(bool bHookEvents)
	: m_bHookEvents(bHookEvents)
	, m_pParent(NULL)
{
}

bool CDesuraWnd::Create(const char* szClassName, const char*, DWORD dwStyl, const CRect& rect, CDesuraWnd *pParent, UINT nID)
{
	m_pParent = pParent;

	m_hWND = ::CreateWindow(szClassName, "", dwStyl | WS_CHILD | WS_VISIBLE, rect.left, rect.top, rect.width(), rect.height(), pParent->GetSafeHwnd(), NULL, GetInstanceHandle(), 0);
	::SetWindowLong(m_hWND, GWL_ID, nID);

	return m_hWND != NULL;
}

bool CDesuraWnd::Create(CDesuraWnd *pParent, const CRect& rect, UINT nID, DWORD dwStyle)
{
	assert(false);
	m_pParent = pParent;
	return false;
}

bool CDesuraWnd::RegisterWindowClass()
{
	return false;
}

CDesuraWnd* CDesuraWnd::GetParent()
{
	return m_pParent;
}

HCURSOR CDesuraWnd::LoadStandardCursor(const char* szResourceId)
{
	return ::LoadCursor(NULL, szResourceId);
}

bool CDesuraWnd::RegisterClass(WNDCLASS *pClass)
{
	return ::RegisterClass(pClass);
}

HINSTANCE CDesuraWnd::gs_hInstance = NULL;

void CDesuraWnd::SetInstanceHandle(HINSTANCE hInstance)
{
	gs_hInstance = hInstance;
}

HINSTANCE CDesuraWnd::GetInstanceHandle()
{
	assert(gs_hInstance);
	return gs_hInstance;
}

void CDesuraWnd::BeginThread(ThreadFn funct, void* pData)
{
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)funct, pData, 0, NULL);
}

void CDesuraWnd::OnPaint()
{
	assert(false);
}

bool CDesuraWnd::OnEraseBkgnd()
{
	assert(false);
	return false;
}

void CDesuraWnd::OnMouseMove(UINT nFlags, CPoint point)
{
	assert(false);
}

void CDesuraWnd::OnLButtonUp(UINT nFlags, CPoint point)
{
	assert(false);
}

void CDesuraWnd::OnLButtonDown(UINT nFlags, CPoint point)
{
	assert(false);
}

void CDesuraWnd::OnMouseLeave()
{
	assert(false);
}



CDesuraDialog* CDesuraDialog::gs_pCurrentDialog = NULL;


CDesuraDialog::CDesuraDialog(int nResourceId)
	: CDesuraWnd(false)
	, m_nResourceId(nResourceId)
{
}

INT_PTR CDesuraDialog::DoModal()
{
	gs_pCurrentDialog = this;
	INT_PTR nRes = ::DialogBoxParam(GetInstanceHandle(), MAKEINTRESOURCE(m_nResourceId), NULL, &CDesuraDialog::WinProc, (LPARAM)SW_SHOW);

	DWORD err = GetLastError();

	return nRes;
}

void CDesuraDialog::EndDialog(int nResult)
{
	gs_pCurrentDialog = NULL;
	::EndDialog(GetSafeHwnd(), nResult);
}

void CDesuraDialog::SetIcon(HICON hIcon, bool bSmall)
{

}

INT_PTR CDesuraDialog::WinProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_INITDIALOG)
		gs_pCurrentDialog->SetHwnd(hDlg);

	switch (message)
	{
	case WM_INITDIALOG:
		gs_pCurrentDialog->OnInitDialog();
		return TRUE;

	case WM_COMMAND:
		if (lParam)
			gs_pCurrentDialog->OnCommand((HWND)lParam, HIWORD(wParam));
		break;

	case WM_CLOSE:
		gs_pCurrentDialog->EndDialog(LOWORD(wParam));
		break;
	}

	return FALSE;
}