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
#include "gcJSItemInfo.h"
#include "MainApp.h"

#include "usercore/ItemInfoI.h"
#include "usercore/ItemHandleI.h"

REGISTER_JSEXTENDER(DesuraJSItemInfo);


/*void FromJSObject(UserCore::Item::ItemInfoI* &item, JSObjHandle& arg)
{
	if (arg->isObject())
		item = arg->getUserObject<UserCore::Item::ItemInfoI>();
	else
		item = NULL;
}*/ // In gcJSBinding.cpp


void FromJSObject(std::map<gcString, gcString> &map, JSObjHandle& arg)
{
	if (arg->isArray() == false && arg->isObject() == false)
		return;

	for (int y=0; y<arg->getNumberOfKeys(); y++)
	{
		char key[255] = {0};
		gcString val;

		arg->getKey(y, key, 255);

		try
		{
			JSObjHandle jObj = arg->getValue(key);
			FromJSObject(val, jObj);
			map[gcString(key)] = val;
		}
		catch (...) // TODO
		{
		}
	}
}

void FromJSObject(std::vector<std::map<gcString, gcString>> &args, JSObjHandle& arg)
{
	if (arg->isArray() == false)
		return;

	for (int x=0; x<arg->getArrayLength(); x++)
	{
		JSObjHandle a = arg->getValue(x);

		std::map<gcString, gcString> m;
		FromJSObject(m, a);

		args.push_back(m);
	}
}

DesuraJSItemInfo::DesuraJSItemInfo() : DesuraJSBase<DesuraJSItemInfo>("item", "native_binding_iteminfo.js")
{
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemId, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemParentId, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemInstalledModId, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemChangedFlags, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemStatus, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemPercent, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemPermissions, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemOptions, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( isItemLaunchable, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( isItemUpdating, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( isItemInstalled, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( isItemDownloadable, DesuraJSItemInfo );

	REG_SIMPLE_JS_OBJ_FUNCTION( getItemGenre, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemRating, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemDev, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemName, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemShortName, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemStatusStr, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemPath, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemInsPrimary, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemIcon, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemLogo, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemIconUrl, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemLogoUrl, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemProfile, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemDevProfile, DesuraJSItemInfo );

	REG_SIMPLE_JS_OBJ_FUNCTION( getItemBranchCount, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemBranchByIndex, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemBranchById, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemCurrentBranch, DesuraJSItemInfo );

	REG_SIMPLE_JS_OBJ_FUNCTION( getItemInstallSize, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemDownloadSize, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemInstalledBuild, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemInstalledBranch, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemInstalledVersion, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( getItemType, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( isItemFavorite, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_VOIDFUNCTION( setItemFavorite, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( isItemValid, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( isLink, DesuraJSItemInfo );

	REG_SIMPLE_JS_OBJ_VOIDFUNCTION( addItemOptionFlags, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_VOIDFUNCTION( delItemOptionFlags, DesuraJSItemInfo );

	REG_SIMPLE_JS_OBJ_FUNCTION( getItemExeInfo, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_VOIDFUNCTION( setItemUserArgs, DesuraJSItemInfo );

	REG_SIMPLE_JS_OBJ_FUNCTION( isItemBusy, DesuraJSItemInfo );
	REG_SIMPLE_JS_OBJ_FUNCTION( createItemDesktopShortcut, DesuraJSItemInfo );
	
	m_uiValidItemHash = UTIL::MISC::RSHash_CSTR("isItemValid");
}

bool DesuraJSItemInfo::preExecuteValidation(const char* function, uint32 functionHash, JSObjHandle object, size_t argc, JSObjHandle* argv)
{
	if (argc == 0)
		return false;

	if (functionHash == m_uiValidItemHash)
		return true;

	if (argv[0]->isObject() == false)
		return false;

	UserCore::Item::ItemInfoI* item = NULL;
	FromJSObject(item, argv[0]);

	return isItemValid(item);
}




bool DesuraJSItemInfo::isLink(UserCore::Item::ItemInfoI* item)
{
	return item->getId().getType() == DesuraId::TYPE_LINK;
}

bool DesuraJSItemInfo::isItemValid(UserCore::Item::ItemInfoI* item)
{
	return item?true:false;
}

gcString DesuraJSItemInfo::getItemId(UserCore::Item::ItemInfoI* item)
{
	return item->getId().toString();
}

gcString DesuraJSItemInfo::getItemParentId(UserCore::Item::ItemInfoI* item)
{
	return item->getParentId().toString();
}

gcString DesuraJSItemInfo::getItemInstalledModId(UserCore::Item::ItemInfoI* item)
{
	return item->getInstalledModId().toString();
}



int32 DesuraJSItemInfo::getItemChangedFlags(UserCore::Item::ItemInfoI* item)
{
	return item->getChangedFlags();
}

int32 DesuraJSItemInfo::getItemStatus(UserCore::Item::ItemInfoI* item)
{
	return item->getStatus();
}

int32 DesuraJSItemInfo::getItemPercent(UserCore::Item::ItemInfoI* item)
{
	return item->getPercent();
}

int32 DesuraJSItemInfo::getItemPermissions(UserCore::Item::ItemInfoI* item)
{
	return item->getPermissions();
}

int32 DesuraJSItemInfo::getItemOptions(UserCore::Item::ItemInfoI* item)
{
	return item->getOptions();
}



bool DesuraJSItemInfo::isItemLaunchable(UserCore::Item::ItemInfoI* item)
{
	return item->isLaunchable();
}

bool DesuraJSItemInfo::isItemUpdating(UserCore::Item::ItemInfoI* item)
{
	return item->isUpdating();
}

bool DesuraJSItemInfo::isItemInstalled(UserCore::Item::ItemInfoI* item)
{
	return item->isInstalled();
}

bool DesuraJSItemInfo::isItemDownloadable(UserCore::Item::ItemInfoI* item)
{
	return item->isDownloadable();
}



gcString DesuraJSItemInfo::getItemGenre(UserCore::Item::ItemInfoI* item)
{
	return item->getGenre();
}

gcString DesuraJSItemInfo::getItemRating(UserCore::Item::ItemInfoI* item)
{
	return item->getRating();
}

gcString DesuraJSItemInfo::getItemDev(UserCore::Item::ItemInfoI* item)
{
	return item->getDev();
}

gcString DesuraJSItemInfo::getItemName(UserCore::Item::ItemInfoI* item)
{
	gcString name = item->getName();
	return name;
}

gcString DesuraJSItemInfo::getItemShortName(UserCore::Item::ItemInfoI* item)
{
	return item->getShortName();
}

gcString DesuraJSItemInfo::getItemStatusStr(UserCore::Item::ItemInfoI* item)
{
	UserCore::Item::ItemHandleI* handle = GetUserCore()->getItemManager()->findItemHandle(item->getId());

	char buff[255] = {0};
	handle->getStatusStr(GetLanguageManager(), buff, 255);

	return buff;
}

gcString DesuraJSItemInfo::getItemPath(UserCore::Item::ItemInfoI* item)
{
	return item->getPath();
}

gcString DesuraJSItemInfo::getItemInsPrimary(UserCore::Item::ItemInfoI* item)
{
	return item->getInsPrimary();
}

gcString DesuraJSItemInfo::getItemIcon(UserCore::Item::ItemInfoI* item)
{
	gcString icon(item->getIcon());
	
#ifdef NIX
	if (icon.size() > 0 && icon[0] != '/')
		icon = gcString("{0}/{1}", UTIL::OS::getCurrentDir(), icon);
#endif
	
	return icon;
}

gcString DesuraJSItemInfo::getItemLogo(UserCore::Item::ItemInfoI* item)
{
	gcString logo = item->getLogo();
	
#ifdef NIX
	if (logo.size() > 0 && logo[0] != '/')
		logo = gcString("{0}/{1}", UTIL::OS::getCurrentDir(), logo);
#endif	
	
	return logo;
}

gcString DesuraJSItemInfo::getItemIconUrl(UserCore::Item::ItemInfoI* item)
{
	gcString logo = item->getIconUrl();
	return logo;
}

gcString DesuraJSItemInfo::getItemLogoUrl(UserCore::Item::ItemInfoI* item)
{
	gcString logo = item->getLogoUrl();
	return logo;
}

gcString DesuraJSItemInfo::getItemProfile(UserCore::Item::ItemInfoI* item)
{
	return item->getProfile();
}

gcString DesuraJSItemInfo::getItemDevProfile(UserCore::Item::ItemInfoI* item)
{
	return item->getDevProfile();
}

gcString DesuraJSItemInfo::getItemInstalledVersion(UserCore::Item::ItemInfoI* item)
{
	return item->getInstalledVersion();
}



int32 DesuraJSItemInfo::getItemBranchCount(UserCore::Item::ItemInfoI* item)
{
	return item->getBranchCount();
}

void* DesuraJSItemInfo::getItemBranchByIndex(UserCore::Item::ItemInfoI* item, int32 index)
{
	return item->getBranch(index);
}

void* DesuraJSItemInfo::getItemBranchById(UserCore::Item::ItemInfoI* item, int32 id)
{
	return item->getBranchById(id);
}

void* DesuraJSItemInfo::getItemCurrentBranch(UserCore::Item::ItemInfoI* item)
{
	return item->getCurrentBranch();
}



int32 DesuraJSItemInfo::getItemInstallSize(UserCore::Item::ItemInfoI* item)
{
	return item->getInstallSize();
}

int32 DesuraJSItemInfo::getItemDownloadSize(UserCore::Item::ItemInfoI* item)
{
	return item->getDownloadSize();
}

int32 DesuraJSItemInfo::getItemInstalledBuild(UserCore::Item::ItemInfoI* item)
{
	return item->getInstalledBuild();
}

int32 DesuraJSItemInfo::getItemInstalledBranch(UserCore::Item::ItemInfoI* item)
{
	return item->getInstalledBranch();
}

int32 DesuraJSItemInfo::getItemType(UserCore::Item::ItemInfoI* item)
{
	return item->getId().getType();
}

bool DesuraJSItemInfo::isItemFavorite(UserCore::Item::ItemInfoI* item)
{
	return item->isFavorite();
}

void DesuraJSItemInfo::setItemFavorite(UserCore::Item::ItemInfoI* item, bool setFav)
{
	item->setFavorite(setFav);
}

void DesuraJSItemInfo::addItemOptionFlags(UserCore::Item::ItemInfoI* item, int32 options)
{
	item->addOFlag(options);
}

void DesuraJSItemInfo::delItemOptionFlags(UserCore::Item::ItemInfoI* item, int32 options)
{
	item->delOFlag(options);
}










std::vector<MapElementI*> DesuraJSItemInfo::getItemExeInfo(UserCore::Item::ItemInfoI* item)
{
	std::vector<MapElementI*> ret;

	std::vector<UserCore::Item::Misc::ExeInfoI*> vExeList;
	item->getExeList(vExeList);

	for (size_t x=0; x<vExeList.size(); x++)
	{
		std::map<gcString, gcString> values;

		values["name"] = vExeList[x]->getName();
		values["exe"] = vExeList[x]->getExe();
		values["exeargs"] = vExeList[x]->getExeArgs();
		values["userargs"] = vExeList[x]->getUserArgs();

		ret.push_back(new MapElement<std::map<gcString, gcString>>(values));
	}

	return ret;
}

void DesuraJSItemInfo::setItemUserArgs(UserCore::Item::ItemInfoI* item, std::vector<std::map<gcString, gcString>> args)
{
	std::vector<UserCore::Item::Misc::ExeInfoI*> vExeList;
	item->getExeList(vExeList);

	for (size_t x=0; x<args.size(); x++)
	{
		gcString name = args[x]["name"];
		gcString userargs = args[x]["userargs"];

		for (size_t y=0; y<vExeList.size(); y++)
		{
			if (name == vExeList[y]->getName())
			{
				vExeList[y]->setUserArgs(userargs.c_str());
				break;
			}
		}
	}
}


bool DesuraJSItemInfo::isItemBusy(UserCore::Item::ItemInfoI* item)
{
	UserCore::Item::ItemHandleI* handle = GetUserCore()->getItemManager()->findItemHandle(item->getId());
	return handle && handle->isInStage();
}

bool DesuraJSItemInfo::createItemDesktopShortcut(UserCore::Item::ItemInfoI* item)
{
	UserCore::Item::ItemHandleI* handle = GetUserCore()->getItemManager()->findItemHandle(item->getId());
	return handle && handle->createDesktopShortcut();
}
