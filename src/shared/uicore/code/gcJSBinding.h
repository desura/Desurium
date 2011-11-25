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

#ifndef DESURA_GCJSBINDING_H
#define DESURA_GCJSBINDING_H
#ifdef _WIN32
#pragma once
#endif

#include "gcJSBase.h"

namespace UserCore
{
	namespace Item
	{
		class ItemInfoI;
	}
}

class DesuraJSBinding : public DesuraJSBase<DesuraJSBinding>
{
public:
	DesuraJSBinding();
	~DesuraJSBinding();

protected:
	JSObjHandle getLocalString(ChromiumDLL::JavaScriptFactoryI *m_pFactory, ChromiumDLL::JavaScriptContextI* context, JSObjHandle object, std::vector<JSObjHandle> &args);

	void* getItemInfoFromId(gcString id);

	std::vector<void*> getDevItems();
	std::vector<void*> getGames();
	std::vector<void*> getMods(UserCore::Item::ItemInfoI* game);
	std::vector<void*> getLinks();
	std::vector<void*> getFavorites();
	std::vector<void*> getRecent();
	std::vector<void*> getUploads();
	std::vector<void*> getNewItems();

	gcString getThemeColor(gcString name, gcString id);
	gcString getThemeImage(gcString id);

	gcString base64encode(gcString data);

	gcString getCacheValue(gcString name, gcString defaultV);
	void setCacheValue(gcString name, gcString value);

	int32 getItemFromId(gcString szId);
	gcString getTypeFromId(gcString szId);

	bool isOffline();

	gcString getCVarValue(gcString name);
	bool isValidIcon(gcString url);

	void updateCounts(int32 msg, int32 updates, int32 threads, int32 cart);
	void forceUpdatePoll();

	bool isWindows();
	bool isLinux();
	bool is32Bit();
	bool is64Bit();

	void* addLink(gcString name, gcString exe, gcString args);
	void delLink(UserCore::Item::ItemInfoI* item);
	void updateLink(UserCore::Item::ItemInfoI* item, gcString args);

	void autoLogin(gcString username, gcString loginCookie);
};


#endif //DESURA_GCJSBINDING_H
