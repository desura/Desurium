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
#include "gcJSBinding.h"
#include "MainApp.h"

#include "gcJSItemInfo.h"
#include "Managers.h"

REGISTER_JSEXTENDER(DesuraJSBinding);

void FromJSObject(UserCore::Item::ItemInfoI* &item, JSObjHandle& arg)
{
	if (!GetUserCore())
		return;

	if (arg->isObject() == false)
		return;

	item = arg->getUserObject<UserCore::Item::ItemInfoI>();
}


DesuraJSBinding::DesuraJSBinding() : DesuraJSBase("app", "native_binding.js")
{
	REGISTER_JS_FUNCTION( getLocalString, DesuraJSBinding );

	
	REG_SIMPLE_JS_FUNCTION( getItemInfoFromId, DesuraJSBinding );
	REG_SIMPLE_JS_FUNCTION( getDevItems, DesuraJSBinding );
	REG_SIMPLE_JS_FUNCTION( getGames, DesuraJSBinding );
	REG_SIMPLE_JS_FUNCTION( getMods, DesuraJSBinding );
	REG_SIMPLE_JS_FUNCTION( getLinks, DesuraJSBinding );

	REG_SIMPLE_JS_FUNCTION( getFavorites, DesuraJSBinding );
	REG_SIMPLE_JS_FUNCTION( getRecent, DesuraJSBinding );
	REG_SIMPLE_JS_FUNCTION( getUploads, DesuraJSBinding );
	REG_SIMPLE_JS_FUNCTION( getNewItems, DesuraJSBinding );


	REG_SIMPLE_JS_FUNCTION( getThemeColor, DesuraJSBinding );
	REG_SIMPLE_JS_FUNCTION( getThemeImage, DesuraJSBinding );
	REG_SIMPLE_JS_FUNCTION( base64encode, DesuraJSBinding );

	REG_SIMPLE_JS_VOIDFUNCTION( setCacheValue, DesuraJSBinding );
	REG_SIMPLE_JS_FUNCTION( getCacheValue, DesuraJSBinding );

	REG_SIMPLE_JS_FUNCTION( getItemFromId, DesuraJSBinding );
	REG_SIMPLE_JS_FUNCTION( getTypeFromId, DesuraJSBinding );

	REG_SIMPLE_JS_FUNCTION( isOffline, DesuraJSBinding );
	REG_SIMPLE_JS_FUNCTION( getCVarValue, DesuraJSBinding );

	REG_SIMPLE_JS_FUNCTION( isValidIcon, DesuraJSBinding );

	REG_SIMPLE_JS_VOIDFUNCTION( updateCounts, DesuraJSBinding );
	REG_SIMPLE_JS_VOIDFUNCTION( forceUpdatePoll, DesuraJSBinding );

	REG_SIMPLE_JS_FUNCTION( isWindows, DesuraJSBinding );
	REG_SIMPLE_JS_FUNCTION( isLinux, DesuraJSBinding );
	REG_SIMPLE_JS_FUNCTION( is32Bit, DesuraJSBinding );
	REG_SIMPLE_JS_FUNCTION( is64Bit, DesuraJSBinding );


	REG_SIMPLE_JS_FUNCTION( addLink, DesuraJSBinding );
	REG_SIMPLE_JS_OBJ_VOIDFUNCTION( delLink, DesuraJSBinding );
	REG_SIMPLE_JS_OBJ_VOIDFUNCTION( updateLink, DesuraJSBinding );

	REG_SIMPLE_JS_VOIDFUNCTION( autoLogin, DesuraJSBinding );
}

DesuraJSBinding::~DesuraJSBinding()
{
}



template <typename T>
class FormatWrapper : public Template::FormatArgC<char, T>
{
public:
	FormatWrapper(T t) : Template::FormatArgC<char, T>(m_tVal), m_tVal(t)
	{
	}

private:
	T m_tVal;
};

JSObjHandle DesuraJSBinding::getLocalString(ChromiumDLL::JavaScriptFactoryI *m_pFactory, ChromiumDLL::JavaScriptContextI* context, JSObjHandle object, std::vector<JSObjHandle> &args)
{
	if (!m_pFactory)
		return NULL;
	
	gcString buff;
	FromJSObject(buff, args[0]);

	const char* str = Managers::GetString(buff.c_str());

	if (args.size() == 1)
		return m_pFactory->CreateString(str);

	std::vector<Template::FormatArgI<char>*> argsList;

	for (size_t x=1; x<args.size(); x++)
	{
		if (args[x]->isBool())
		{
			argsList.push_back(new FormatWrapper<bool>(args[x]->getBoolValue()));
		}
		else if (args[x]->isDouble())
		{
			argsList.push_back(new FormatWrapper<double>(args[x]->getDoubleValue()));
		}
		else if (args[x]->isInt())
		{
			argsList.push_back(new FormatWrapper<int32>(args[x]->getIntValue()));
		}
		else if (args[x]->isString())
		{
			char buff[255] = {0};
			args[x]->getStringValue(buff, 255);

			argsList.push_back(new FormatWrapper<std::string>(buff));
		}
		else
		{
			Warning(gcString("Bad type for getLocalString format in arg {0} [{1}]\n", x, str));
		}
	}

	gcString outStr = Template::FormatString(str, argsList);

	for (size_t x=0; x<argsList.size(); x++)
		argsList[x]->destroy();

	return m_pFactory->CreateString(outStr.c_str()); 
}

void* DesuraJSBinding::getItemInfoFromId(gcString szId)
{
	if (!GetUserCore() || !GetUserCore()->getItemManager())
		return NULL;

	DesuraId id(UTIL::MISC::atoll(szId.c_str()));
	return GetUserCore()->getItemManager()->findItemInfo(id);
}

std::vector<void*> DesuraJSBinding::getDevItems()
{
	std::vector<void*> ret;

	if (!GetUserCore() || !GetUserCore()->getItemManager())
		return ret;

	std::vector<UserCore::Item::ItemInfoI*> gList;
	GetUserCore()->getItemManager()->getDevList(gList);

	for (size_t x=0; x<gList.size(); x++)
		ret.push_back(gList[x]);

	return ret;
}

std::vector<void*> DesuraJSBinding::getGames()
{
	std::vector<void*> ret;

	if (!GetUserCore() || !GetUserCore()->getItemManager())
		return ret;

	std::vector<UserCore::Item::ItemInfoI*> gList;
	GetUserCore()->getItemManager()->getGameList(gList);

	for (size_t x=0; x<gList.size(); x++)
		ret.push_back(gList[x]);

	return ret;
}

std::vector<void*> DesuraJSBinding::getMods(UserCore::Item::ItemInfoI* game)
{
	std::vector<void*> ret;

	if (!game || !GetUserCore() || !GetUserCore()->getItemManager())
		return ret;

	std::vector<UserCore::Item::ItemInfoI*> mList;
	GetUserCore()->getItemManager()->getModList(game->getId(), mList);

	for (size_t x=0; x<mList.size(); x++)
		ret.push_back(mList[x]);

	return ret;
}

std::vector<void*> DesuraJSBinding::getLinks()
{
	std::vector<void*> ret;

	if (!GetUserCore() || !GetUserCore()->getItemManager())
		return ret;

	std::vector<UserCore::Item::ItemInfoI*> gList;
	GetUserCore()->getItemManager()->getLinkList(gList);

	for (size_t x=0; x<gList.size(); x++)
		ret.push_back(gList[x]);

	return ret;
}

std::vector<void*> DesuraJSBinding::getFavorites()
{
	std::vector<void*> ret;

	if (!GetUserCore() || !GetUserCore()->getItemManager())
		return ret;

	std::vector<UserCore::Item::ItemInfoI*> gList;
	GetUserCore()->getItemManager()->getFavList(gList);

	for (size_t x=0; x<gList.size(); x++)
		ret.push_back(gList[x]);

	return ret;
}

std::vector<void*> DesuraJSBinding::getRecent()
{
	std::vector<void*> ret;

	if (!GetUserCore() || !GetUserCore()->getItemManager())
		return ret;

	std::vector<UserCore::Item::ItemInfoI*> gList;
	GetUserCore()->getItemManager()->getRecentList(gList);

	for (size_t x=0; x<gList.size(); x++)
		ret.push_back(gList[x]);

	return ret;
}

std::vector<void*> DesuraJSBinding::getUploads()
{
	std::vector<void*> ret;

	if (!GetUploadMng())
		return ret;

	size_t count = GetUploadMng()->getCount();

	for (size_t x=0; x<count; x++)
	{
		UserCore::Misc::UploadInfoThreadI* item = GetUploadMng()->getItem(x);

		if (item->isDeleted())
			continue;

		ret.push_back(item);
	}

	return ret;
}

std::vector<void*> DesuraJSBinding::getNewItems()
{
	std::vector<void*> ret;

	if (!GetUserCore() || !GetUserCore()->getItemManager())
		return ret;

	std::vector<UserCore::Item::ItemInfoI*> gList;
	GetUserCore()->getItemManager()->getNewItems(gList);

	for (size_t x=0; x<gList.size(); x++)
		ret.push_back(gList[x]);

	return ret;
}





gcString DesuraJSBinding::getThemeColor(gcString name, gcString id)
{
	Color col = GetGCThemeManager()->getColor(name.c_str(), id.c_str());

	char colStr[255];
	Safe::snprintf(colStr, 255, (char*)"#%02X%02X%02X", col.red, col.green, col.blue);

	return gcString(colStr);
}

gcString DesuraJSBinding::getThemeImage(gcString id)
{
	gcString img = GetGCThemeManager()->getImage(id.c_str());
	gcString path = GetGCThemeManager()->getThemeFolder();

	size_t pos = img.find(path);
	if (pos != std::string::npos)
	{
		img = gcString(".") + img.substr(pos + path.size(), -1);
	}

	return img;
}

gcString DesuraJSBinding::base64encode(gcString data)
{
	return UTIL::STRING::base64_encode(data.c_str(), data.size());
}








std::vector<CVar*> g_pCvarList;
AutoDeleteV<std::vector<CVar*>> cvad(g_pCvarList);




gcString DesuraJSBinding::getCacheValue(gcString name, gcString defaultV)
{
	gcString fullname("WEBCACHE_{0}", name);

	CVar* cvar = GetCVarManager()->findCVar(fullname.c_str());
	if (!cvar)
	{
		cvar = new CVar(fullname.c_str(), gcString(defaultV).c_str(), CFLAG_USER);
		g_pCvarList.push_back(cvar);
	}

	return cvar->getString();
}

void DesuraJSBinding::setCacheValue(gcString name, gcString value)
{
	gcString fullname("WEBCACHE_{0}", name);
	CVar* cvar = GetCVarManager()->findCVar(fullname.c_str());

	if (!cvar)
	{
		cvar = new CVar(fullname.c_str(), "", CFLAG_USER);
		g_pCvarList.push_back(cvar);
	}

	cvar->setValue(value.c_str());
}



int32 DesuraJSBinding::getItemFromId(gcString szId)
{
	DesuraId id(UTIL::MISC::atoll(szId.c_str()));
	return id.getItem();
}

gcString DesuraJSBinding::getTypeFromId(gcString szId)
{
	DesuraId id(UTIL::MISC::atoll(szId.c_str()));
	return id.getTypeString();
}

bool DesuraJSBinding::isOffline()
{
	return g_pMainApp->isOffline();
}

gcString DesuraJSBinding::getCVarValue(gcString name)
{
	CVar* cvar = GetCVarManager()->findCVar(name.c_str());

	if (!cvar)
		return "";

	return cvar->getString();
}

const char* g_ValidImageList[] = 
{
	"png",
	"gif",
	"jpg",
	"jpeg",
	NULL,
};

bool DesuraJSBinding::isValidIcon(gcString url)
{
	UTIL::FS::Path path = UTIL::FS::PathWithFile(url);

	if (!UTIL::FS::isValidFile(path))
		return false;

	bool found = false;

	gcString ext = path.getFile().getFileExt();
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	size_t x=0;

	while (!found && g_ValidImageList[x])
	{
		if (ext == g_ValidImageList[x])
			found = true;

		x++;
	}

	return found;
}

void DesuraJSBinding::updateCounts(int32 msg, int32 updates, int32 threads, int32 cart)
{
	if (GetUserCore())
		GetUserCore()->setCounts(msg, updates, threads, cart);
}

void DesuraJSBinding::forceUpdatePoll()
{
	if (g_pMainApp)
		g_pMainApp->handleInternalLink("desura://refresh/background");
}

bool DesuraJSBinding::isWindows()
{
#if defined WIN32 || defined WIN64
	return true;
#else
	return false;
#endif
}

bool DesuraJSBinding::isLinux()
{
#ifdef NIX
	return true;
#else
	return false;
#endif
}

bool DesuraJSBinding::is32Bit()
{
	return !UTIL::OS::is64OS();
}

bool DesuraJSBinding::is64Bit()
{
	return UTIL::OS::is64OS();
}


CONCOMMAND( cc_addlink, "gc_link_add" )
{
	if (vArgList.size() < 4)
	{
		Warning("Need args: [name] [exe] [args]\n");
	}
	else
	{
		DesuraId id = GetUserCore()->getItemManager()->addLink(vArgList[1].c_str(), vArgList[2].c_str(), vArgList[3].c_str());
		Msg(gcString("Added link id: {0}\n", id.toInt64()));
	}
}

void* DesuraJSBinding::addLink(gcString name, gcString exe, gcString args)
{
	if (name == "" || !UTIL::FS::isValidFile(exe))
		return NULL;

	if (!GetUserCore() || !GetUserCore()->getItemManager())
		return NULL;

	DesuraId id = GetUserCore()->getItemManager()->addLink(name.c_str(), exe.c_str(), args.c_str());
	return GetUserCore()->getItemManager()->findItemInfo(id);
}

void DesuraJSBinding::delLink(UserCore::Item::ItemInfoI* item)
{
	if (item->getId().getType() != DesuraId::TYPE_LINK)
		return;

	if (!GetUserCore() || !GetUserCore()->getItemManager())
		return;

	GetUserCore()->getItemManager()->removeItem(item->getId());	
}

void DesuraJSBinding::updateLink(UserCore::Item::ItemInfoI* item, gcString args)
{
	if (!GetUserCore() || !GetUserCore()->getItemManager())
		return;

	GetUserCore()->getItemManager()->updateLink(item->getId(), args.c_str());
}

void DesuraJSBinding::autoLogin(gcString username, gcString loginCookie)
{
	g_pMainApp->newAccountLogin(username.c_str(), loginCookie.c_str());
}