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
#include "Courgette.h"
#include "SharedObjectLoader.h"
#include "util_thread/BaseThread.h"
#include "BZip2.h"

#include "CourgetteI.h"

typedef void* (*FactoryFn)(const char*);

class Courgette
{
public:
	Courgette()
	{
		m_uiInstanceCount = 0;
	}

	CourgetteI* newInstance()
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
			throw gcException(ERR_LIBRARY_LOADFAILED, "Failed to load courgette");

		return (CourgetteI*)m_pFactory(COURGETTE);
	}

	void destroyInstance(CourgetteI* courgette)
	{
		if (!courgette)
			return;

		courgette->destory();

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
#ifdef WIN32
		if (!m_ScriptCore.load("courgette_desura.dll"))
			return false;
#else
		if (!m_ScriptCore.load("libcourgette_desura.so"))
			return false;
#endif

		m_pFactory = m_ScriptCore.getFunction<FactoryFn>("FactoryBuilderCourgette");

		if (!m_pFactory)
			return false;

		return true;
	}

	void unloadDll()
	{
		m_pFactory = NULL;
		m_ScriptCore.unload();
		m_ScriptCore = SharedObjectLoader();
	}

private:
	uint32 m_uiInstanceCount;
	FactoryFn m_pFactory;

	SharedObjectLoader m_ScriptCore;
	::Thread::Mutex m_ScriptLock;
};

Courgette g_Courgette;


class CourgetteCallback : public CourgetteCallbackI
{
public:
	CourgetteCallback(CourgetteWriteCallbackI* writeCb)
	{
		m_pWriteCb = writeCb;
	}

	virtual bool write(const char* data, size_t size)
	{
		if (size == 0)
			return true;

		if (m_pWriteCb)
			return m_pWriteCb->writeData(data, size);

		return false;
	}

private:
	CourgetteWriteCallbackI* m_pWriteCb;
};


class CourgettBuffer : public CourgetteBufferI
{
public:
	CourgettBuffer(const char* buff, size_t size)
	{
		this->buff = buff;
		this->size = size;
	}

	virtual const char* getBuffer()
	{
		return buff;
	}

	virtual size_t getSize()
	{
		return size;
	}

private:
	const char* buff;
	size_t size;
};

class CourgettFileBuffer : public CourgetteBufferI
{
public:
	CourgettFileBuffer(const char* file)
	{
		buff = NULL;
		size = UTIL::FS::readWholeFile(file, &buff);
	}

	~CourgettFileBuffer()
	{
		safe_delete(buff);
	}

	virtual const char* getBuffer()
	{
		return buff;
	}

	virtual size_t getSize()
	{
		return size;
	}

private:
	char* buff;
	size_t size;
};


CourgetteInstance::CourgetteInstance()
{
	m_pCourgette = NULL;

	try
	{
		m_pCourgette = g_Courgette.newInstance();
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to create courgette instance: {0}\n", e));
	}
}

CourgetteInstance::~CourgetteInstance()
{
	if (m_pCourgette)
		g_Courgette.destroyInstance(m_pCourgette);
}

bool CourgetteInstance::createDiffCB(const char* buffOld, size_t sizeOld, const char* buffNew, size_t sizeNew, CourgetteWriteCallbackI* callback)
{
	if (!m_pCourgette || !buffOld || !buffNew)
		return false;

	CourgetteCallback cb(callback);

	CourgettBuffer oldBuff(buffOld, sizeOld);
	CourgettBuffer newBuff(buffNew, sizeNew);

	return m_pCourgette->createDiff(&oldBuff, &newBuff, &cb);
}

bool CourgetteInstance::applyDiffCB(const char* fileOld, const char* fileDiff, CourgetteWriteCallbackI* callback)
{
	if (!m_pCourgette || !fileOld || !fileDiff)
		return false;

	gcString pathOld = fileOld;
	gcString pathDiff = fileDiff;

	if (!UTIL::FS::isValidFile(pathOld) || !UTIL::FS::isValidFile(pathDiff))
		return false;

	CourgetteCallback cb(callback);

	CourgettFileBuffer oldBuff(fileOld);
	CourgettFileBuffer diffBuff(fileDiff);

	return m_pCourgette->applyDiff(&oldBuff, &diffBuff, &cb);
}
