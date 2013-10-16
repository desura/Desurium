#pragma once

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

#include <windows.h>
#include <cassert>

namespace Desurium
{
	class CSize : public SIZE
	{
	public:
		CSize()
		{
			cx = 0;
			cy = 0;
		}

		CSize(SIZE &size)
		{
			cx = size.cx;
			cy = size.cy;
		}
	};

	class CPoint : public POINT
	{
	public:
		CPoint()
		{
			x = 0;
			y = 0;
		}

		CPoint(int x, int y)
		{
			this->x = x;
			this->y = y;
		}

		CPoint operator-(const CPoint& point) const
		{
			return CPoint(this->x - point.x, this->y - point.y);
		}
	};

	class CRect : public RECT
	{
	public:
		CRect()
		{
			left = 0;
			top = 0;
			right = 0;
			bottom = 0;
		}

		CRect(int x, int y, int w, int h)
		{
			left = x;
			top = y;
			right = x + w;
			bottom = y + h;
		}

		bool PtInRect(const CPoint& point) const
		{
			return point.x > left && point.x < right && point.y > top && point.y < bottom;
		}

		int width() const
		{
			return right-left;
		}

		int height() const
		{
			return bottom - top;
		}
	};

	class CDesuraWnd;
	class CDC;

	class CBitmap
	{
	public:
		CBitmap();
		~CBitmap();

		void LoadBitmap(int nResourceId);
		void CreateCompatibleBitmap(CDC &dc, int w, int h);
		int GetBitmap(BITMAP *pBitmap);

		operator HGDIOBJ ()
		{
			return m_hBitmap;
		}

	private:
		HBITMAP m_hBitmap;
	};

	class CFont
	{
	public:
		CFont()
			: m_hFont(NULL)
		{
		}

		~CFont()
		{
			DeleteObject(m_hFont);
		}

		void CreatePointFont(int nSize, const char* szName)
		{
			m_hFont = CreateFont(nSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, szName);
		}

		operator HFONT ()
		{
			return m_hFont;
		}

	private:
		HFONT m_hFont;
	};


	class CDC
	{
	public:
		CDC();
		CDC(HDC hdc);
		~CDC();

		void SetBkMode(int nMode);
		void SetTextColor(COLORREF nColor);
		void CreateCompatibleDC(CDC* pDC);

		HGDIOBJ SelectObject(CBitmap* pBitmap);
		HGDIOBJ SelectObject(HGDIOBJ pObject);
		HGDIOBJ SelectObject(CFont* pFont);

		void StretchBlt(int x, int y, int nWidth, int nHeight, CDC* pSrcDC, int xSrc, int ySrc, int nSrcWidth, int nSrcHeight, DWORD dwRop);
		void BitBlt(int x, int y, int nWidth, int nHeight, CDC* pSrcDC, int xSrc, int ySrc, DWORD dwRop);

		CSize GetOutputTextExtent(const char* szText, int nSize);
		bool TextOut(int x, int y, const char* szText, int nSize);

		operator HDC ()
		{
			return m_hDC;
		}

	private:
		HDC m_hDC;
	};

	class CPaintDC : public CDC
	{
	public:
		CPaintDC(CDesuraWnd* pWnd);
		~CPaintDC();

	private:
		CDesuraWnd *m_pWnd;
		PAINTSTRUCT m_lpPaint;
	};




	typedef UINT (__stdcall *ThreadFn)(void*);

	class CDesuraWnd
	{
	public:
		CDesuraWnd(bool bHookEvents);

		virtual bool Create(const char* szClassName, const char*, DWORD dwStyl, const CRect& rect, CDesuraWnd *pParent, UINT nID);
		virtual bool Create(CDesuraWnd *pParent, const CRect& rect, UINT nID, DWORD dwStyle = WS_VISIBLE);
		virtual bool RegisterWindowClass();

		BOOL RedrawWindow(const RECT *lprcUpdate, HRGN hrgnUpdate, UINT flags)
		{
			assert(m_hWND);
			return ::RedrawWindow(m_hWND, lprcUpdate, hrgnUpdate, flags);
		}

		int SetWindowRgn(HRGN hRgn, BOOL bRedraw)
		{
			assert(m_hWND);
			return ::SetWindowRgn(m_hWND, hRgn, bRedraw);
		}

		HICON LoadIcon(int nResourceId)
		{
			return ::LoadIcon(GetInstanceHandle(), MAKEINTRESOURCE(nResourceId));
		}

		HWND SetCapture()
		{
			assert(m_hWND);
			return ::SetCapture(m_hWND);
		}

		void ReleaseCapture()
		{
			assert(m_hWND);
			::ReleaseCapture();
		}

		BOOL SetWindowPos(int x, int y, int cx, int cy, UINT uFlags)
		{
			assert(m_hWND);
			return ::SetWindowPos(m_hWND, NULL, x, y, cx, cy, uFlags);
		}

		void ClientToScreen(CPoint &point)
		{
			assert(m_hWND);
			::ClientToScreen(m_hWND, &point);
		}

		void GetClientRect(CRect &rect)
		{
			assert(m_hWND);
			::GetClientRect(m_hWND, &rect);
		}

		HWND GetSafeHwnd()
		{
			return m_hWND;
		}

		HDC GetDC()
		{
			assert(m_hWND);
			return ::GetDC(m_hWND);
		}

		CDesuraWnd* GetParent();

		HCURSOR LoadStandardCursor(const char* szResourceId);

		bool RegisterClass(WNDCLASS *pClass);

		static void SetInstanceHandle(HINSTANCE hInstance);
		static HINSTANCE GetInstanceHandle();
		static void BeginThread(ThreadFn funct, void* pData);

	protected:
		virtual void OnPaint();
		virtual bool OnEraseBkgnd();
		virtual void OnMouseMove(UINT nFlags, CPoint point);
		virtual void OnLButtonUp(UINT nFlags, CPoint point);
		virtual void OnLButtonDown(UINT nFlags, CPoint point);
		virtual void OnMouseLeave();

		void SetHwnd(HWND hWnd)
		{
			m_hWND = hWnd;
		}

	private:
		bool m_bHookEvents;
		HWND m_hWND;

		static HINSTANCE gs_hInstance;

		CDesuraWnd *m_pParent;
	};


	class CDesuraDialog : public CDesuraWnd
	{
	public:
		CDesuraDialog(int nResourceId);

		virtual bool OnInitDialog()=0;

		virtual INT_PTR DoModal();
		virtual void EndDialog(int nResult);

		virtual void OnCommand(HWND hWnd, int nId)
		{
		}

	protected:
		void SetIcon(HICON hIcon, bool bSmall);

		static INT_PTR CALLBACK WinProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	private:
		static CDesuraDialog* gs_pCurrentDialog;
		
		int m_nResourceId;
	};


}