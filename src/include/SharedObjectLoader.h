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

#ifndef DESURA_SHAREDOBJECTLOADER_H
#define DESURA_SHAREDOBJECTLOADER_H
#ifdef _WIN32
#pragma once
#endif

typedef void* (*FactoryFn)(const char*);

#ifdef WIN32
	#include <Windows.h>
#else
	#include <dlfcn.h>
#endif

class SharedObjectLoader
{
public:
#ifdef NIX
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

#ifdef NIX
		m_hHandle = dlopen(module, RTLD_NOW);

		if (!m_hHandle)
			fprintf(stderr, "%s:%d - Error loading library %s: '%s' [LD_LIBRARY_PATH=%s]\n", __FILE__, __LINE__, module, dlerror(), getenv("LD_LIBRARY_PATH"));
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
		if (dlclose(m_hHandle) != 0)
			printf("%s:%d - Error unloading library: '%s'\n", __FILE__, __LINE__, dlerror());
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
		char* error;
		T fun = (T)dlsym(m_hHandle, functionName);
		if ((error = dlerror()) != NULL)
		{
			printf("%s:%d - Error getting function %s: '%s'\n", __FILE__, __LINE__, functionName, dlerror());
			return NULL;
		}

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
	mutable SOHANDLE m_hHandle;
};

#endif //DESURA_SHAREDOBJECTLOADER_H
