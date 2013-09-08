/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
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

#include "Common.h"
#include "ToolManager.h"

#include "ScriptCoreI.h"
#include "gcJSBase.h"

#include "User.h"


extern LogCallback* g_pLogCallback;

class UserItem : public ScriptCoreItemI
{
public:
	virtual void destroy()
	{
		delete this;
	}

	UserCore::Item::ItemInfo* m_pItem;
	UserCore::ToolManager* m_pToolManager;
	UserCore::Item::BranchInfoI* m_pBranch;
};

void FromJSObject(UserItem* &jsItem, JSObjHandle& arg)
{
	if (arg->isObject())
		jsItem = dynamic_cast<UserItem*>(arg->getUserObject<ScriptCoreItemI>());
	else
		jsItem = NULL;
}



class JSToolInfo : public UserCore::ToolInfo
{
public:
	JSToolInfo(DesuraId toolId) : UserCore::ToolInfo(toolId)
	{
	}

	JSToolInfo(DesuraId itemId, MCFBranch branchId, DesuraId toolId, gcString name, gcString exe, gcString args, gcString res) : UserCore::ToolInfo(toolId)
	{
		m_ItemId = itemId;
		m_uiBranchId = branchId;

		m_szNameString = name;
		m_szExe = exe;
		m_szArgs = args;
		m_szResult = res;
	}

	virtual bool isDownloaded()
	{
		return true;
	}

	virtual bool isInstalled()
	{
		//need this here as a bug in desura deleted the tool exe after running (GOD FUCK!) and if it dont exist just pretend its installed.
		if (!checkExePath(m_szExe.c_str()))
			return true;

		return UserCore::ToolInfo::isInstalled();
	}

	DesuraId getItemId()
	{
		return m_ItemId;
	}

	MCFBranch getBranchId()
	{
		return m_uiBranchId;
	}

	virtual void setExePath(const char* exe)
	{
	}

protected:
	virtual void loadFromDb(sqlite3x::sqlite3_connection* db)
	{
		ToolInfo::loadFromDb(db);

		std::vector<std::string> out;
		UTIL::STRING::tokenize(m_szUrl, out, "|");

		m_ItemId = DesuraId(UTIL::MISC::atoll(out[0].c_str()));

		if (out.size() > 1)
			m_uiBranchId = MCFBranch::BranchFromInt(atoi(out[1].c_str()));

		m_szUrl = "";
	}

	virtual void saveToDb(sqlite3x::sqlite3_connection* db)
	{
		m_szUrl = gcString("{0}|{1}", m_ItemId.toString(), (uint32)m_uiBranchId);
		ToolInfo::saveToDb(db);

		m_szUrl = "";
	}

	virtual bool checkExePath(const char* path, bool quick = false)
	{
		return (path && UTIL::FS::isValidFile(path));
	}

	MCFBranch m_uiBranchId;
	DesuraId m_ItemId;
};

UserCore::ToolInfo* NewJSToolInfo(DesuraId id)
{
	return new JSToolInfo(id);
}




namespace UserCore
{


class ItemExtender : public DesuraJSBase<ItemExtender>
{
public:
	ItemExtender() :  DesuraJSBase("item", "installer_binding_item_user.js")
	{
		REG_SIMPLE_JS_OBJ_FUNCTION( GetInstallPath, ItemExtender );
		REG_SIMPLE_JS_OBJ_VOIDFUNCTION( AddTool, ItemExtender );
	}

	gcString GetInstallPath(UserItem* item)
	{
		if (!item)
			return "NULL ITEM";

		gcString p = item->m_pItem->getPath();
		p.push_back(UTIL::FS::Path::GetDirSeperator());

		return p;
	}

	void AddTool(UserItem* item, gcString name, gcString exe, gcString args, gcString res)
	{
		if (!item)
			return;

		if (name.size() == 0 || exe.size() == 0)
			return;

		item->m_pToolManager->addJSTool(item->m_pItem, item->m_pBranch->getBranchId(), name, exe, args, res);
	}
};

ItemExtender g_ItemExtender;








void ToolManager::addJSTool(UserCore::Item::ItemInfo* item, uint32 branchId, gcString name, gcString exe, gcString args, gcString res)
{
	if (!item)
		return;

	UserCore::Item::BranchInfoI* branch = item->getBranchById(branchId);

	if (!branch)
		return;

	UserCore::Item::BranchInfo* realBranch = dynamic_cast<UserCore::Item::BranchInfo*>(branch);

	if (!realBranch)
		return;

	bool found = false;

	BaseManager<ToolInfo>::for_each([&](ToolInfo* info)
	{
		JSToolInfo* jsinfo = dynamic_cast<JSToolInfo*>(info);

		if (!jsinfo)
			return;

		if (item->getId() == jsinfo->getItemId() && name == info->getName() && jsinfo->getBranchId() == branchId)
			found = true;
	});

	if (found)
		return;

	DesuraId toolId(m_iLastCustomToolId, DesuraId::TYPE_TOOL);
	m_iLastCustomToolId--;

	JSToolInfo* tool = new JSToolInfo(item->getId(), realBranch->getBranchId(), toolId, name, exe, args, res);
	realBranch->addJSTool(toolId);
	addItem(tool);
}

void ToolManager::findJSTools(UserCore::Item::ItemInfo* item)
{
	bool validPath = false;

	for (size_t x=0; x<item->getBranchCount(); x++)
	{
		gcString path = item->getBranch(x)->getInstallScriptPath();

		if (UTIL::FS::isValidFile(path))
			validPath = true;
	}

	if (!validPath)
		return;

	m_ScriptLock.lock();
	
	if (!m_pFactory)
	{
		Warning("Failed to load scriptcore for find JS Tools\n");
		m_ScriptLock.unlock();
		return;
	}

	m_uiInstanceCount++;
	m_ScriptLock.unlock();

	for (size_t x=0; x<item->getBranchCount(); x++)
	{
		gcString path = item->getBranch(x)->getInstallScriptPath();

		if (!UTIL::FS::isValidFile(path))
			continue;

		ScriptCoreI* instance = (ScriptCoreI*)m_pFactory(SCRIPT_CORE);

		UserItem* userItem = new UserItem();
		userItem->m_pItem = item;
		userItem->m_pBranch = item->getBranch(x);
		userItem->m_pToolManager = this;

		try
		{
			instance->setItem(userItem);
			instance->executeScript(path.c_str());
			instance->executeString("ToolSetup();");
		}
		catch (gcException &e)
		{
			gcString errMsg(e.getErrMsg());

			if (errMsg.find("ToolSetup is not defined") == std::string::npos)
				Warning(gcString("Failed to execute toolsetup: {0}\n", e));
		}

		instance->destory();
	}

	m_ScriptLock.lock();
	m_uiInstanceCount--;
	m_ScriptLock.unlock();

	if (m_uiInstanceCount == 0)
		unloadJSEngine();
}

bool ToolManager::initJSEngine()
{
	bool res = false;

	m_ScriptLock.lock();

	if (m_uiInstanceCount == 0 && !m_pFactory)
		res = loadJSEngine();
	else
		res = true;

	if (res)
		m_uiInstanceCount++;

	m_ScriptLock.unlock();

	if (!res)
	{
#ifdef WIN32
		Warning(gcString("Failed to load scriptcore.dll: {0}\n", GetLastError()));
#else
		Warning(gcString("Failed to load libscriptcore.so: {0}\n", dlerror()));
#endif
		unloadJSEngine();
	}

	return res;
}

bool ToolManager::loadJSEngine()
{
#ifdef WIN32
	if (!m_ScriptCore.load("scriptcore.dll"))
		return false;
#else
	if (!m_ScriptCore.load("libscriptcore.so"))
		return false;
#endif

	m_pFactory = m_ScriptCore.getFunction<FactoryFn>("FactoryBuilderScriptCore");

	if (!m_pFactory)
		return false;

	RegLogFn regLog = m_ScriptCore.getFunction<RegLogFn>("RegDLLCB_SCRIPT");

	if (regLog)
		regLog(g_pLogCallback);
	else
#ifdef WIN32
		Warning("Failed to setup logging for scriptcore.dll\n");
#else
		Warning("Failed to setup logging for scriptcore.so\n");
#endif

	ScriptCoreSetupI* setup = (ScriptCoreSetupI*)m_pFactory(SCRIPT_CORE_SETUP);

	if (!setup)
		return false;

	m_tJSEngineExpireTime = time(NULL) + 60*15;
	setup->useInternalTaskRunner();

	return setup->addItemExtender(&g_ItemExtender);
}

void ToolManager::destroyJSEngine()
{
	m_ScriptLock.lock();
	m_uiInstanceCount--;
	m_ScriptLock.unlock();

	if (m_uiInstanceCount == 0)
		unloadJSEngine();
}

void ToolManager::unloadJSEngine(bool forced)
{
	m_ScriptLock.lock();

	time_t now = time(NULL);

	if (forced || (m_uiInstanceCount == 0 && now > m_tJSEngineExpireTime))
	{
		if (m_pFactory)
		{
			ScriptCoreSetupI* setup = (ScriptCoreSetupI*)m_pFactory(SCRIPT_CORE_SETUP);

			if (setup)
				setup->cleanUp();
		}

		m_pFactory = NULL;
		m_ScriptCore.unload();
		m_ScriptCore = SharedObjectLoader();

		m_uiInstanceCount = 0;
	}

	m_ScriptLock.unlock();
}

}
