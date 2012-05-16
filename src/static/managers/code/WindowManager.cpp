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

#include "Common.h"
#include "WindowManager.h"
#include <wx/wx.h>

namespace Managers
{

void LoadTheme(wxWindow* win, const char* name)
{
	if (!GetThemeManager())
		return;

	Color bg = GetThemeManager()->getColor(name, "bg");
	Color fg = GetThemeManager()->getColor(name, "fg");

	win->SetForegroundColour(wxColor(fg));
	win->SetBackgroundColour(wxColor(bg));
}

}


WindowItem::WindowItem(wxFrame* frame) : BaseItem()
{
	m_uiHash = frame->GetId();
	m_pFrame = frame;
}

WindowManager::WindowManager() : BaseManager<WindowItem>(true)
{

}

void WindowManager::registerWindow(wxFrame* win)
{
	if (win)
		addItem(new WindowItem(win));
}

void WindowManager::unRegisterWindow(wxFrame* win)
{
	if (win)
		removeItem(win->GetId(), true);
}

void WindowManager::getWindowList(std::vector<wxFrame*> &vList)
{
	for (uint32 x=0; x<getCount(); x++)
		vList.push_back(getItem(x)->m_pFrame);

	std::sort(vList.begin(), vList.end(), [](wxFrame* lhs, wxFrame* rhs){
		return wcscmp(lhs->GetTitle().c_str(), rhs->GetTitle().c_str())>0;
	});
}

