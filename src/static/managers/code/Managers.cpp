/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Jookia <166291@gmail.com>
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
#include "managers/Managers.h"

#include "ThemeManager.h"
#include "LanguageManager.h"
#include "WindowManager.h"

#include "CVarManager.h"
#include "ConCommandManager.h"

namespace Managers
{


const char* GetString( const char* str )
{
	return GetLanguageManager().getString( str );
}

const wchar_t* GetString( const wchar_t* str )
{
	return GetLanguageManager().getString( str );
}

}

/**
 * a simple thread-safe static storage SingletonHolder implementation
 */
template <typename Class>
class SingletonHolder
{
public:
	static Class &Instance();

private:
	static Class* instance;
	static QuickMutex mutex;

	static void NewInstance();

	// should be never ever called
	SingletonHolder();
};

template <typename Class>
inline Class &SingletonHolder<Class>::Instance()
{
	// create instance only if instance is not created
	if (!instance)
		NewInstance();

	return *instance;
}

template <typename Class>
inline void SingletonHolder<Class>::NewInstance()
{
	// enter critical section
	mutex.lock();

	// check again for creation (another thread could accessed the critical section before
	if (!instance)
		instance = new Class();

	mutex.unlock();
}

template <typename Class>
Class *SingletonHolder<Class>::instance = nullptr;

template <typename Class>
QuickMutex SingletonHolder<Class>::mutex;

class ManagersImpl
{
public:
	void DestroyManagers()
	{
		DestroyCVarManager();
		DestroyConComManager();
	}

	void InitManagers()
	{
		InitCVarManger();
		InitConComManger();
	}
};

ManagersImpl g_Managers;

void InitManagers()
{
	g_Managers.InitManagers();
}

void DestroyManagers()
{
	g_Managers.DestroyManagers();
}

LanguageManagerI & GetLanguageManager()
{
	return SingletonHolder<LanguageManager>::Instance();
}

ThemeManagerI & GetThemeManager()
{
	return SingletonHolder<ThemeManager>::Instance();
}

WindowManagerI & GetWindowManager()
{
	return SingletonHolder<WindowManager>::Instance();
}

CVarManagerI* GetCVarManager()
{
	return g_pCVarMang;
}

CCommandManagerI* GetCCommandManager()
{
	return g_pConComMang;
}
