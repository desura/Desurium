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
#include "ScriptCore.h"
#include "ScriptCoreInternal.h"

#include "jsItem.h"
#include "jsFS.h"
#include "jsWin.h"
#include <iostream>

#include "util_thread/BaseThread.h"



typedef void (*MessageFunct)(const char* message);

RegisterJSExtenderHelper<FileSystemJSBinding> regFS;
RegisterJSExtenderHelper<ItemJSBinding> regItem;
RegisterJSExtenderHelper<OsJSBinding> regOS;



ScriptCoreI* NewScriptCore(ScriptTaskRunnerI* runner)
{
	//cause we are in a lib, these will get filtered out if we dont use them. :(
	regFS.noOp();
	regItem.noOp();
	regOS.noOp();

	return new ScriptCore(runner);
}




class ScriptTask : public ScriptTaskI
{
public:
	ScriptTask(ScriptCoreInternal* scriptInternal)
	{
		m_pScriptCore = scriptInternal;
	}

	~ScriptTask()
	{
		m_DelLock.lock();
		m_DelLock.unlock();
	}

	virtual void doTask()
	{
		try
		{
			run();
		}
		catch (gcException &e)
		{
			m_Exception = e;
		}

		m_DelLock.lock();
		m_Wait.notify();
		m_DelLock.unlock();
	}

	virtual void destory()
	{
		m_WaitDestory.notify();
	}

	void wait()
	{
		m_Wait.wait();
	}

	void waitForDestroy()
	{
		m_WaitDestory.wait();
	}

	void checkRes()
	{
		if (m_Exception.getErrId() > 1)
			throw m_Exception;
	}

protected:
	virtual void run()=0;
	ScriptCoreInternal* m_pScriptCore;

private:
	gcException m_Exception;
	::Thread::WaitCondition m_Wait;
	::Thread::WaitCondition m_WaitDestory;
	::Thread::Mutex m_DelLock;
};

class InitV8 : public ScriptTask
{
public:
	InitV8(ScriptCoreInternal* scriptInternal) : ScriptTask(scriptInternal)
	{
	}

	virtual void run()
	{
		m_pScriptCore->init();
	}
};

class DelV8 : public ScriptTask
{
public:
	DelV8(ScriptCoreInternal* scriptInternal) : ScriptTask(scriptInternal)
	{
	}

	virtual void run()
	{
		m_pScriptCore->del();
	}
};

class RunScript : public ScriptTask
{
public:
	RunScript(ScriptCoreInternal* scriptInternal, const char* file, char* buff, uint32 size) : ScriptTask(scriptInternal), m_AutoDelBuff(buff)
	{
		m_szFile = file;
		m_uiSize = size;
	}

	virtual void run()
	{
		m_pScriptCore->runScript(m_szFile.c_str(), m_AutoDelBuff, m_uiSize);
	}

	uint32 m_uiSize;
	AutoDelete<char> m_AutoDelBuff;
	gcString m_szFile;
};


class RunString : public ScriptTask
{
public:
	RunString(ScriptCoreInternal* scriptInternal, const char* string) : ScriptTask(scriptInternal)
	{
		m_szString = string;
	}

	virtual void run()
	{
		m_pScriptCore->runString(m_szString.c_str());
	}

	gcString m_szString;
};




ScriptCore::ScriptCore(ScriptTaskRunnerI* runner)
{
	m_pTaskRunner = NULL;
	m_uiItemId = -1;
	m_pTaskRunner = runner;

	m_pInternal = new ScriptCoreInternal();

	runTask(new InitV8(m_pInternal));
}

ScriptCore::~ScriptCore()
{
	runTask(new DelV8(m_pInternal));

	safe_delete(m_pInternal);
	delItem();
}

void ScriptCore::delItem()
{
	if (m_uiItemId == UINT_MAX)
		return;

	ScriptCoreItemI* temp = ItemJSBinding::GetItem(m_uiItemId);
	ItemJSBinding::RemoveItem(m_uiItemId);

	if (temp)
		temp->destroy();

	m_uiItemId = -1;
}

void ScriptCore::setItem(ScriptCoreItemI* item)
{
	if (!item)
		return;

	ScriptCoreItemI* temp = ItemJSBinding::GetItem(m_uiItemId);

	if (temp == item)
		return;

	delItem();

	m_uiItemId = ItemJSBinding::AddItem(item);
	executeString(gcString("item.SetItem({0});", (int32)m_uiItemId).c_str());
}

void ScriptCore::runTask(ScriptTask* task)
{
	if (m_pTaskRunner)
	{
		m_pTaskRunner->queTask(task);
		task->wait();
	}
	else
	{
		task->doTask();
	}

	task->checkRes();
	task->waitForDestroy();
	delete task;
}

void ScriptCore::executeScript(const char* file)
{
	if (!file || !UTIL::FS::isValidFile(file))
		throw gcException(ERR_INVALID, "File does not exist");

	char* out = NULL;
	size_t size = UTIL::FS::readWholeFile(file, &out);

	runTask(new RunScript(m_pInternal, file, out, size));
}

void ScriptCore::executeString(const char* string)
{
	runTask(new RunString(m_pInternal, string));
}
