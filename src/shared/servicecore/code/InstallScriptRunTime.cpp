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
#include "InstallScriptRunTime.h"
#include "ScriptCoreI.h"

#include "SharedObjectLoader.h"
#include "util_thread/BaseThread.h"

#include "Log.h"

typedef void* (*FactoryFn)(const char*);
typedef void (*RegLogFn)(LogCallback*);

#include "gcJSBase.h"

#ifdef WIN32
#include <ShlObj.h>
#endif


gcString GetSpecialPath(int32 key);

class ServiceItem : public ScriptCoreItemI
{
public:
	virtual void destroy()
	{
		delete this;
	}

	gcString m_szInstallPath;
};

void FromJSObject(ServiceItem* &jsItem, JSObjHandle& arg)
{
	if (arg->isObject())
		jsItem = dynamic_cast<ServiceItem*>(arg->getUserObject<ScriptCoreItemI>());
	else
		jsItem = NULL;
}

class ItemExtender : public DesuraJSBase<ItemExtender>
{
public:
	ItemExtender() :  DesuraJSBase<ItemExtender>("item", "installer_binding_item_service.js")
	{
		REG_SIMPLE_JS_OBJ_FUNCTION( GetInstallPath, ItemExtender );
		REG_SIMPLE_JS_FUNCTION( GetSpecialPath, ItemExtender );
		REG_SIMPLE_JS_FUNCTION( GetWildcardPath, ItemExtender );
	}

	gcString GetInstallPath(ServiceItem* item)
	{
		if (item)
			return item->m_szInstallPath;

		return "[NULL ITEM]";
	}

	gcString GetSpecialPath(int32 key)
	{
		return ::GetSpecialPath(key);
	}

	gcString GetWildcardPath(gcString wildcard)
	{
#ifdef WIN32
		if (Safe::stricmp("PROGRAM_FILES", wildcard.c_str()) == 0)
		{
			wchar_t path[MAX_PATH]  = {0};
			SHGetFolderPathW(NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, path);
			return path;
		}
		else if (Safe::stricmp("DOCUMENTS", wildcard.c_str()) == 0)
		{
			return GetSpecialPath(1);
		}
		else if (Safe::stricmp("JAVA_PATH", wildcard.c_str()) == 0)
		{
			std::string cur = UTIL::WIN::getRegValue("HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Runtime Environment\\CurrentVersion");

			if (cur.size() > 0)
				return UTIL::WIN::getRegValue(gcString("HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Runtime Environment\\{0}\\JavaHome", cur));
		}
		else if (Safe::stricmp("APP_DATA", wildcard.c_str())==0)
		{
			wchar_t path[MAX_PATH]  = {0};
			SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);
			return path;
		}
		else if (Safe::stricmp("USER_APP_DATA", wildcard.c_str())==0)
		{
			return GetSpecialPath(2);
		}
#endif
		return "";
	}
};

class ScriptCore
{
public:
	ScriptCore()
	{
		m_uiInstanceCount = 0;
		m_pFactory = NULL;
		m_pSetup = NULL;
	}

	ScriptCoreI* newInstance()
	{
		bool loaded = false;

		m_ScriptLock.lock();

		if (m_uiInstanceCount == 0)
			loaded = loadDll();
		else
			loaded = true;

		bool didLoad = (loaded && m_pFactory);

		if (didLoad)
			m_uiInstanceCount++;
		m_ScriptLock.unlock();

		if (didLoad == false)
			throw gcException(ERR_LIBRARY_LOADFAILED, "Failed to load script engine");

		return (ScriptCoreI*)m_pFactory(SCRIPT_CORE);
	}

	void destroyInstance(ScriptCoreI* scriptCore)
	{
		if (!scriptCore)
			return;

		scriptCore->destory();

		m_ScriptLock.lock();

		if (m_uiInstanceCount > 0)
			m_uiInstanceCount--;

		if (m_uiInstanceCount == 0)
			unloadDll();

		m_ScriptLock.unlock();
	}

protected:
	bool loadDll()
	{
#if !defined(DEBUG) && defined(DESURA_OFFICIAL_BUILD) && defined(WITH_CODESIGN)
	#ifdef WIN32
			char message[255] = {0};
			if (UTIL::WIN::validateCert(L".\\bin\\scriptcore.dll", message, 255) != ERROR_SUCCESS)
			{
				Warning(gcString("Cert validation failed on scriptcore.dll: {0}\n", message));
				return false;
			}
	#endif
#endif

#ifdef WIN32
		if (!m_ScriptCore.load("scriptcore.dll"))
			return false;
#else
		if (!m_ScriptCore.load("libscriptcore.so"))
			return false;
#endif

		m_pFactory = m_ScriptCore.getFunction<FactoryFn>("FactoryBuilderScriptCore");

		if (!m_pFactory)
		{
			unloadDll();
			return false;
		}

#ifndef IGNORE_DESURA_LOGGING
		RegLogFn regLog = m_ScriptCore.getFunction<RegLogFn>("RegDLLCB_SCRIPT");

		if (regLog)
			InitLogging(regLog);
		else
			Warning("Failed to setup logging for scriptcore\n");
#endif

		m_pSetup = (ScriptCoreSetupI*)m_pFactory(SCRIPT_CORE_SETUP);

		if (!m_pSetup)
		{
			unloadDll();
			return false;
		}

		m_pSetup->useInternalTaskRunner();

		return m_pSetup->addItemExtender(&m_Extender);
	}

	void unloadDll()
	{
		if (m_pSetup)
			m_pSetup->cleanUp();

		m_pSetup = NULL;
		m_pFactory = NULL;
		m_ScriptCore.unload();
		m_ScriptCore = SharedObjectLoader();
	}

private:
	uint32 m_uiInstanceCount;
	FactoryFn m_pFactory;

	ScriptCoreSetupI* m_pSetup;

	SharedObjectLoader m_ScriptCore;
	::Thread::Mutex m_ScriptLock;

	ItemExtender m_Extender;
};

ScriptCore g_ScriptCore;

InstallScriptRunTime::InstallScriptRunTime(const char* scriptFile, const char* installPath)
{
	m_pRunTime = NULL;
	loadScript(scriptFile, installPath);
}

InstallScriptRunTime::~InstallScriptRunTime()
{
	if (m_pRunTime)
		g_ScriptCore.destroyInstance(m_pRunTime);
}

void InstallScriptRunTime::loadScript(const char* scriptFile, const char* installPath)
{
	if (!UTIL::FS::isValidFile(scriptFile))
		return;

	bool failed = false;

	try
	{
		m_pRunTime = g_ScriptCore.newInstance();

		if (!m_pRunTime)
			throw gcException(ERR_NULLHANDLE, "Failed to create script instance!");

		ServiceItem* item = new ServiceItem();
		item->m_szInstallPath = installPath;

		m_pRunTime->setItem(item);
		m_pRunTime->executeScript(scriptFile);
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to execute install script {0}: {1}\n", scriptFile, e));
		failed = true;
	}

	if (failed)
	{
		if (m_pRunTime)
			g_ScriptCore.destroyInstance(m_pRunTime);

		m_pRunTime = NULL;
	}
}

void InstallScriptRunTime::run(const char* function)
{
	if (!m_pRunTime)
		return;

	gcString f("{0}();", function);

	try
	{
		m_pRunTime->executeString(f.c_str());
	}
	catch (gcException &e)
	{
		gcString errMsg(e.getErrMsg());

		if (errMsg.find(gcString("{0} is not defined", function)) == std::string::npos)
			throw;
	}
}
