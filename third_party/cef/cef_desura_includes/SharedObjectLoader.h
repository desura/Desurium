///////////// Copyright © 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : mcf_util
//   File        : SharedObjectLoader.h
//   Description :
//      [TODO: Write the purpose of SharedObjectLoader.h.]
//
//   Created On: 7/18/2010 5:49:15 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef DESURA_SHAREDOBJECTLOADER_H
#define DESURA_SHAREDOBJECTLOADER_H
#ifdef _WIN32
#pragma once
#endif

#ifdef WIN32
	#include <Windows.h>
#else
    #include <dlfcn.h>
#endif

#ifdef NIX
#define OS_LINUX
#endif

class SharedObjectLoader
{
public:
#ifdef OS_LINUX
    typedef void* SOHANDLE;
#else
	typedef HINSTANCE SOHANDLE;
#endif

	SharedObjectLoader()
	{
		m_hHandle = NULL;
		m_bHasFailed = false;
	}

	SharedObjectLoader(const SharedObjectLoader& sol)
	{
		m_hHandle = sol.m_hHandle;
		m_bHasFailed = sol.m_bHasFailed;

		sol.m_hHandle = NULL;
		sol.m_bHasFailed = false;
	}

	~SharedObjectLoader()
	{
		unload();
	}

	bool load(const char* module)
	{
		if (m_hHandle)
			unload();

		m_bHasFailed = false;

#ifdef OS_LINUX
		m_hHandle = dlopen(module, RTLD_LAZY);
#else
		m_hHandle = LoadLibraryA(module);
#endif
		return m_hHandle?true:false;
	}

	void unload()
	{
		if (!m_hHandle)
			return;

#ifdef NIX
		dlclose(m_hHandle);
#else
		FreeLibrary(m_hHandle);
#endif

		m_hHandle = NULL;
	}

	template <typename T>
	T getFunction(const char* functionName)
	{
		if (!m_hHandle)
			return NULL;
#ifdef NIX
		T fun = (T)dlsym(m_hHandle, functionName);
#else
		T fun = (T)GetProcAddress(m_hHandle, functionName);
#endif

		if (!fun)
			m_bHasFailed = true;

		return fun;
	}

	bool hasFailed()
	{
		return m_bHasFailed;
	}

	SOHANDLE handle()
	{
		return m_hHandle;
	}

private:
	mutable bool m_bHasFailed;
#ifdef WIN32
	mutable SOHANDLE m_hHandle;
#else
	mutable void* m_hHandle;
#endif
};

#endif //DESURA_SHAREDOBJECTLOADER_H
