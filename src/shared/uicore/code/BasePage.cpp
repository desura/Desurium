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
#include "BasePage.h"

#include "MainApp.h"
#include "usercore/ItemInfoI.h"
#include "usercore/ItemManagerI.h"

BasePage::BasePage( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style ) : gcPanel( parent, id, pos, size, style )
{
	m_uiMCFBuild = 0;
	m_uiMCFBranch = 0;
}

void BasePage::setInfo(DesuraId id)
{
	m_iInternId = id;

	UserCore::Item::ItemInfoI *item = GetUserCore()->getItemManager()->findItemInfo(id);

	if (item && item->getIcon() && UTIL::FS::isValidFile(UTIL::FS::PathWithFile(item->getIcon())))
	{
		gcFrame *frame = dynamic_cast<gcFrame*>(GetParent());
		
		if (frame)
			frame->setIcon(item->getIcon());
	}
}

void BasePage::setMCFInfo(MCFBranch branch, MCFBuild build)
{
	m_uiMCFBuild = build;
	m_uiMCFBranch = branch;
}

UserCore::Item::ItemInfoI* BasePage::getItemInfo()
{
	return GetUserCore()->getItemManager()->findItemInfo(m_iInternId);
}
