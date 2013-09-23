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

#include "DesuraWinApp.h"
#include <windows.h>

#include <cassert>

using namespace Desurium;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	CDesuraWinApp& app = CDesuraWinApp::GetApp();

	app.Init(hInstance, nCmdShow, lpCmdLine);

	app.InitInstance();
	return app.ExitInstance();
}


CDesuraWinApp* CDesuraWinApp::gs_pWinApp = NULL;


CDesuraWinApp::CDesuraWinApp()
	: m_hInstance(NULL)
	, m_nCmdShow(0)
	, m_lpCmdLine(NULL)
{
	gs_pWinApp = this;
}

CDesuraWinApp::~CDesuraWinApp()
{
	if (gs_pWinApp == this)
		gs_pWinApp = NULL;
}

void CDesuraWinApp::Init(HINSTANCE hInstance, int nCmdShow, const char* lpCmdLine)
{
	m_hInstance = hInstance;
	m_nCmdShow = nCmdShow;
	m_lpCmdLine = lpCmdLine;
}

CDesuraWinApp& CDesuraWinApp::GetApp()
{
	if (!gs_pWinApp)
	{
		assert(false);
		abort();
	}
		
	return *gs_pWinApp;
}