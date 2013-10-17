/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Ian T. Jacobsen <iantj92@gmail.com>
          (C) Karol Herbst <git@karolherbst.de>

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

#ifndef DESURA_WILDCARD_MANAGER_H
#define DESURA_WILDCARD_MANAGER_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseManager.h"
#include "Event.h"

enum
{
	WCM_OK = 0,
	WCM_ERR_BADXML,
	WCM_ERR_MAXDEPTH,
	WCM_ERR_FAILEDRESOVELD,
	WCM_ERR_FAILEDFIND,
	WCM_ERR_NULLPATH,
};

class WCSpecialInfo
{
public:
	WCSpecialInfo()
	{
		handled = false;
		processed = false;
	}

	gcString name;
	gcString result;
	volatile bool handled;	//true = special wildcard resolved
	volatile bool processed;	//true = finished processing
};

class WildcardInfo : public BaseItem
{
public:
	WildcardInfo(const char* name, const char* path, const char* type, bool resolved = false) : BaseItem(name),
		m_szName(name),
		m_szPath(path),
		m_szType(type),
		m_bResolved(resolved)
	{}

	~WildcardInfo()
	{
	}

	gcString m_szName;
	gcString m_szPath;
	gcString m_szType;

	bool m_bResolved;
};

//! Stores wild cards (path and special locations) and allows them to be resolved into full paths on the file system
class CEXPORT WildcardManager : public BaseManager<WildcardInfo>
{
public:
	WildcardManager();
	WildcardManager(WildcardManager* mng);
	~WildcardManager();

	void load();
	void unload();

	//! used to update INSTALL_PATH and PARENT_INSTALL_PATH wildcards
	void updateInstallWildcard(const char* name, const char* value);

	//! converts wildcards into there properpath. Res needs to be a null char* pointer
	//! 
	//! @param path Path to search for wild cards
	//! @param res Out path
	//! @param fixPath fix slashes to be os
	//!
	void constructPath(const char* path, char **res, bool fixPath = true);

	//! this parses an xml feed.
	uint8 parseXML(tinyxml2::XMLNode* node);

	uint32 getDepth(){return m_uiDepth;}

	Event<WCSpecialInfo> onNeedSpecialEvent;
	Event<WCSpecialInfo> onNeedInstallSpecialEvent;

	void compactWildCards();

protected:

	//! Triggers the event to resolve the wild card
	void needSpecial(WCSpecialInfo *info);

	//! Make sure its a valid wildcard
	bool wildcardCheck(const char* string);

	//! Recursive version of above
	void constructPath(const char* path, char **res, uint8* depth);

	//!
	void resolveWildCard(WildcardInfo *temp);

private:
	uint32 m_uiDepth;
};

#endif //DESURA_WILDCARD_MANAGER_H
