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

#ifndef DESURA_GAMEEXPLORERMANAGER_H
#define DESURA_GAMEEXPLORERMANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseManager.h"
#include "usercore/ItemInfoI.h"

namespace sqlite3x
{
	class sqlite3_connection;
}

namespace UserCore
{
class UserI;

namespace Item
{
	class ItemInfo;
}

namespace Misc
{
	class GameExplorerInfo : public BaseItem
	{
	public:
		enum
		{
			FLAG_INSTALLED = 1,
			FLAG_NEEDSUPATE = 2,
		};

		GameExplorerInfo(DesuraId id, UserCore::UserI* user);
		~GameExplorerInfo();

		void loadFromDb(sqlite3x::sqlite3_connection *db);
		void saveToDb(sqlite3x::sqlite3_connection *db);

		void generateDll();
		void installDll();
		void removeDll();

		bool isInstalled();
		bool needsUpdate();
		bool isInstallable();
		bool needsInstall();

	protected:
		gcWString generateXml();
		gcString generateGuid();

		void regEvent();

		void translateIco(HANDLE handle, const char* icoPath);
		void onInfoChanged(UserCore::Item::ItemInfoI::ItemInfo_s &info);

	protected:
		gcString m_szDllPath;
		gcString m_szGuid;
		uint32 m_uiFlags;

		DesuraId m_Id;
		UserCore::UserI* m_pUser;
		UserCore::Item::ItemInfoI* m_pItemInfo;
	};
}


class GameExplorerManager : public BaseManager<Misc::GameExplorerInfo>
{
public:
	GameExplorerManager(UserI* user);

	void addItem(DesuraId item);
	void removeItem(DesuraId item);

	void loadItems();
	void saveItems();

protected:
	bool shouldInstallItems();

private:
	UserI* m_pUser;

};


}


#endif //DESURA_GAMEEXPLORERMANAGER_H
