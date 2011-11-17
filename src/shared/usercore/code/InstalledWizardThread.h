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

#ifndef DESURA_MODWIZARDTHREAD_H
#define DESURA_MODWIZARDTHREAD_H
#ifdef _WIN32
#pragma once
#endif

namespace UserCore
{
	namespace Misc
	{
		class InstallInfo;
	}
}
class WildcardManager;

#include "MCFThread.h"

namespace UserCore
{

namespace Item
{
	class ItemTaskGroup;
}

namespace Thread
{

//! Installed wizard looks for items allready installed on user computer
//!
class InstalledWizardThread : public MCFThread
{
public:
	InstalledWizardThread();
	virtual ~InstalledWizardThread();

protected:
	void doRun();

	void parseItemsQuick(TiXmlNode *fNode);

	void parseItems1(TiXmlNode *fNode, WildcardManager *pWildCard, std::map<uint64, TiXmlElement*> *vMap = NULL);
	void parseItems2(TiXmlNode *fNode, WildcardManager *pWildCard);

	bool selectBranch(UserCore::Item::ItemInfoI *item);
	void onItemFound(UserCore::Item::ItemInfoI *item);

	void onGameFound(UserCore::Misc::InstallInfo &game);
	void onModFound(UserCore::Misc::InstallInfo &mod);

	std::vector<UserCore::Misc::InstallInfo> m_vGameList;

	bool m_bTriggerNewEvent;


	void triggerProgress();

	void parseGameQuick(TiXmlElement* game);

	void parseGame(DesuraId id, TiXmlElement* game, WildcardManager *pWildCard, TiXmlElement* info = NULL);
	void parseMod(DesuraId parId, DesuraId id, TiXmlElement* mod, WildcardManager *pWildCard, TiXmlElement* info = NULL);

private:
	UserCore::User* m_pUser;
	UserCore::Item::ItemTaskGroup *m_pTaskGroup;

	uint32 m_uiDone;
	uint32 m_uiTotal;

	gcString m_szDbName;
};

}
}

#endif //DESURA_MODWIZARDTHREAD_H
