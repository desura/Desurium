/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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
#include "TBI_ModMenu.h"

#include "wx_controls/gcCustomMenu.h"
#include "MainApp.h"

wxMenu* TBIModMenu::createMenu(uint32 &lastMenuId)
{
	gcMenu* menu = new gcMenu();
	m_IdMapList.clear();

	UserCore::UserI* user = GetUserCore();

	if (!user)
		return menu;

	std::vector<UserCore::Item::ItemInfoI*> gList;
	user->getItemManager()->getGameList(gList);

	std::sort(gList.begin(), gList.end(), [](UserCore::Item::ItemInfoI* left, UserCore::Item::ItemInfoI* right){
		return strcmp(left->getName(), right->getName()) <= 0;
	});

	for (size_t x=0; x<gList.size(); x++)
	{
		UserCore::Item::ItemInfoI *game = gList[x];

		std::vector<UserCore::Item::ItemInfoI*> mList;
		user->getItemManager()->getModList(game->getId(), mList);

		if (mList.size() == 0)
			continue;

		gcMenu* gameMenu = new gcMenu();
		
		std::sort(mList.begin(), mList.end(), [](UserCore::Item::ItemInfoI* left, UserCore::Item::ItemInfoI* right){
			return strcmp(left->getName(), right->getName()) <= 0;
		});

		for (size_t y=0; y<mList.size(); y++)
		{
			UserCore::Item::ItemInfoI *mod = mList[y];

			wxMenuItem* menuItem = new gcMenuItem(gameMenu, lastMenuId, mod->getName());
			gameMenu->Append(menuItem);

			m_IdMapList[lastMenuId] = mod->getId();
			loadIcon(menuItem, mod->getIcon());

			lastMenuId++;
		}

		wxMenuItem* gameItem = new gcMenuItem(gameMenu, wxID_ANY, game->getName(), "", wxITEM_NORMAL, gameMenu);
		loadIcon(gameItem, game->getIcon());

		menu->Append(gameItem);
	}

	if (menu->GetMenuItemCount() == 0)
	{
		wxMenuItem* noItems = new gcMenuItem(menu, lastMenuId, Managers::GetString(L"#TB_NOITEMS"));
		lastMenuId++;

		menu->Append(noItems);
		noItems->Enable(false);
	}

	return menu;
}

void TBIModMenu::onMenuSelect(wxCommandEvent& event)
{
	if (m_IdMapList.find(event.GetId()) != m_IdMapList.end())
		g_pMainApp->handleInternalLink(m_IdMapList[event.GetId()], ACTION_LAUNCH);
}

const wchar_t* TBIModMenu::getMenuName()
{
	return Managers::GetString(L"#TB_MODS");
}
