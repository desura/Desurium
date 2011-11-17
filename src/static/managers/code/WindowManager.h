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

#ifndef DESURA_WINDOWMANAGER_H
#define DESURA_WINDOWMANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseManager.h"
#include "managers/Managers.h"

class wxFrame;


class WindowItem : public BaseItem
{
public:
	WindowItem(wxFrame* frame);
	wxFrame* m_pFrame;
};

class CEXPORT WindowManager : public BaseManager<WindowItem>, public WindowManagerI
{
public:
	WindowManager();

	virtual void registerWindow(wxFrame* win);
	virtual void unRegisterWindow(wxFrame* win);

	virtual void getWindowList(std::vector<wxFrame*> &vList);
};


#endif //DESURA_WINDOWMANAGER_H
