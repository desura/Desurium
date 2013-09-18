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

#pragma once


// DesuraProgress

enum
{
	MODE_STARTUP,
	MODE_DOWNLOADING,
	MODE_INSTALLING,
};

class CBitmap;

class DesuraProgress : public CWnd
{
public:
	DesuraProgress();
	virtual ~DesuraProgress();

	BOOL Create(CWnd* pParentWnd, const RECT& rect, UINT nID, DWORD dwStyle = WS_VISIBLE);

	void setTotal(int total){m_iTotal=total;}
	void setDone(int done);
	int getDone();
	void setProgress(int prog){m_iProg = prog;refresh();}
	void setMode(int mode){m_iMode = mode; m_iProg = 0;refresh();}

	void refresh()
	{
		RedrawWindow(0,0, RDW_INTERNALPAINT|RDW_NOERASE|RDW_UPDATENOW|RDW_INVALIDATE);
	}

protected:
	void OnPaint();
	BOOL OnEraseBkgnd(CDC* pDC);
	BOOL RegisterWindowClass();

	void OnMouseMove(UINT nFlags, CPoint point);
	void OnLButtonUp(UINT nFlags, CPoint point);
	void OnLButtonDown(UINT nFlags, CPoint point);
	void OnMouseLeave();

	CRect getCancelRect();


private:
	int m_iTotal;
	int m_iProg;
	int m_iMode;
	bool m_bMouseOver;

	int m_vDone[3];

	CBitmap *m_pBackgroundImage;
	CBitmap *m_pLoadingBar;

	bool m_bDragging;
	CPoint m_StartPoint;

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNAMIC(DesuraProgress)
};


