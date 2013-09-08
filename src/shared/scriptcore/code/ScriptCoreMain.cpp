/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Jookia <166291@gmail.com>

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
#include <branding/scriptcore_version.h>

#include "LogBones.cpp"
#include "v8.h"

#include "ScriptCoreI.h"
#include "ScriptTaskThread.h"

gcString g_szSCVersion("{0}.{1}.{2}.{3}", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILDNO, VERSION_EXTEND);

ScriptCoreI* NewScriptCore(ScriptTaskRunnerI* taskRunner);
bool AddItemExtender(ChromiumDLL::JavaScriptExtenderI* extender);
bool IsV8Init();

class ScriptCoreSetup : public ScriptCoreSetupI, public ScriptTaskI
{
public:
	ScriptCoreSetup()
	{
		m_bCleanUp = false;
		m_pThread = NULL;
	}

	~ScriptCoreSetup()
	{
		safe_delete(m_pThread);
	}

	virtual bool addItemExtender(ChromiumDLL::JavaScriptExtenderI* extender)
	{
		return AddItemExtender(extender);
	}

	virtual void useInternalTaskRunner()
	{
		if (m_pThread)
			return;

		m_pThread = new ScriptTaskThread();
		m_pThread->start();

		setTaskRunner(m_pThread);
	}

	virtual void setTaskRunner(ScriptTaskRunnerI* runner)
	{
		m_pTaskRunner = runner;

		if (m_pTaskRunner)
			m_pTaskRunner->setLastTask(this);
	}

	virtual void cleanUp()
	{
		m_bCleanUp = true;

		if (m_pThread)
			m_pThread->stop();
	}

	virtual void* newScriptCore()
	{
		return static_cast<void*>(NewScriptCore(m_pTaskRunner));
	}

	virtual void doTask()
	{
		if (m_bCleanUp && IsV8Init())
			v8::V8::Dispose();

		m_bCleanUp = false;
	}

	virtual void destory()
	{
	}

	bool m_bCleanUp;
	ScriptTaskRunnerI* m_pTaskRunner;
	ScriptTaskThread* m_pThread;
};

ScriptCoreSetup g_Setup;

namespace SCore
{
	CEXPORT void* FactoryBuilder(const char* name)
	{
		if (strcmp(name, SCRIPT_CORE)==0)
		{
			return g_Setup.newScriptCore();
		}
		else if (strcmp(name, SCRIPT_CORE_SETUP)==0)
		{
			return (ScriptCoreSetupI*)&g_Setup;
		}

		return NULL;
	}
}

extern "C"
{

CEXPORT void* FactoryBuilderScriptCore(const char* name)
{
	return SCore::FactoryBuilder(name);
}

CEXPORT const char* GetMCFCoreVersion()
{
	return g_szSCVersion.c_str();
}

CEXPORT void RegDLLCB_SCRIPT(LogCallback* cb)
{
	g_pLogCallback = cb;

	if (g_pLogCallback)
		g_pLogCallback->Msg("ScriptCore Logging Started.\n");
}

}





