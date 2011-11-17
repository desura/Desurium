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


#ifndef DESURA_SCRIPTCORE_H
#define DESURA_SCRIPTCORE_H
#ifdef _WIN32
#pragma once
#endif

#include "ScriptCoreI.h"
#include "v8.h"

class ScriptTask;
class ScriptCoreInternal;

class ScriptCore : public ScriptCoreI
{
public:
	ScriptCore(ScriptTaskRunnerI* runner);
	~ScriptCore();

	virtual void setItem(ScriptCoreItemI* item);
	virtual void delItem();

	virtual void executeScript(const char* file);
	virtual void executeString(const char* string);

	virtual void destory()
	{
		delete this;
	}

protected:
	void doRunScript(v8::Handle<v8::Script> script);

	friend class InitV8;
	void init();

	void runTask(ScriptTask* task);

private:
	
	uint32 m_uiItemId;

	ScriptCoreInternal* m_pInternal;
	ScriptTaskRunnerI* m_pTaskRunner;
};




#endif //DESURA_SCRIPTCORE_H
