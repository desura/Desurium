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

#include <wtypes.h>

namespace Desurium
{

	class CDesuraWinApp
	{
	public:
		CDesuraWinApp();
		~CDesuraWinApp();

		virtual void InitInstance()=0;
		virtual int ExitInstance()=0;

		static CDesuraWinApp& GetApp();

		void Init(HINSTANCE hInstance, int nCmdShow, const char* lpCmdLine);

	protected:
		HINSTANCE m_hInstance;
		int m_nCmdShow;
		const char* m_lpCmdLine;

	private:
		static CDesuraWinApp* gs_pWinApp;
	};


}