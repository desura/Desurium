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
#include "managers/CVar.h"
#include "CVarManager.h"

CVar::CVar(const char* name, const char* defVal, int32 flags, CVarCallBackFn callBack) : BaseItem(name)
{
	m_cbCallBack = callBack;
	m_cbCallBackUser = NULL;

	init(name, defVal, flags);
}

CVar::CVar(const char* name, const char* defVal, int32 flags, CVarUserCallBackFn callBack) : BaseItem(name)
{
	m_cbCallBackUser = callBack;
	m_cbCallBack = NULL;

	init(name, defVal, flags);
}

CVar::~CVar()
{
	if (g_pCVarMang && m_bReg)
		g_pCVarMang->UnRegCVar(this);
}

void CVar::init(const char* name, const char* defVal, int32 flags)
{
	m_pUserData = NULL;

	m_szData = defVal;
	m_szDefault = defVal;
	m_iFlags = flags;

	m_bInCallback = false;
	m_bReg = false;

#ifdef WIN32
	if (flags&CVAR_LINUX_ONLY)
		return;
#else
	if (flags&CVAR_WINDOWS_ONLY)
		return;
#endif	

	if (!g_pCVarMang)
		g_pCVarMang = new CVarManager();

	m_bReg = g_pCVarMang->RegCVar(this);

	if (!m_bReg)
		Warning(gcString("Failed to register cvar [{0}] (maybe duplicate)\n", name));		
}

void CVar::checkOsValid() const
{
	//if this asserts, means your using a cvar thats only meant for a different platform.
#ifdef DEBUG
#ifdef WIN32
	if (m_iFlags&CVAR_LINUX_ONLY)
		assert(true);
#else
	if (m_iFlags&CVAR_WINDOWS_ONLY)
		assert(true);
#endif	
#endif
}

bool CVar::getBool() const
{
	checkOsValid();

	if (Safe::stricmp(m_szData.c_str(), "true")==0 || Safe::stricmp(m_szData.c_str(), "1")==0)
		return true;

	return false;
}

int32 CVar::getInt() const
{
	checkOsValid();
	return atoi(m_szData.c_str());
}

float CVar::getFloat() const
{
	checkOsValid();
	return (float)atof(m_szData.c_str());
}

const char* CVar::getExitString()
{
	checkOsValid();

	if (m_szExitData.length() > 0)
		return m_szExitData.c_str();
	else
		return getString();
}

const char* CVar::getString() const
{
	checkOsValid();

	if (m_szData.length() == 0 || m_szData[0] == '\0')
		return NULL;

	return m_szData.c_str();
}

const char* CVar::getDefault() const
{
	checkOsValid();
	return m_szDefault.c_str();
}

void CVar::setDefault()
{
	setValue(m_szDefault.c_str());
}

void CVar::setValue(bool b)
{
	setValue(gcString("{0}", b).c_str());
}

void CVar::setValue(int32 i)
{
	setValue(gcString("{0}", i).c_str());
}

void CVar::setValue(float f)
{
	setValue(gcString("{0}", f).c_str());
}

void CVar::setValue(const char* s)
{
	checkOsValid();

	if (m_iFlags & CFLAG_SAVEONEXIT)
		m_szExitData = s;
	else
		setValueOveride(s);
}

void CVar::setValueOveride(const char* s)
{
	if (s && strcmp(s, m_szData.c_str()) == 0)
		return;

	//we are in the call back all ready. Accept value to stop infint loop
	if (m_bInCallback) 
	{
		m_szData = s;
		return;
	}

	bool res = true;

	m_bInCallback = true;

	if (m_cbCallBack)
		res = m_cbCallBack(this, s);

	else if (m_cbCallBackUser)
		res = m_cbCallBackUser(this, s, m_pUserData);

	m_bInCallback = false;

	if (res)
		m_szData = s;
}

void CVar::parseCommand(std::vector<gcString> &vArgList)
{
	if (vArgList.size() > 1)
	{
		Msg(gcString("CVAR: {0}\n", m_szName));
		Msg(gcString("  Old Value: {0}\n", m_szData));
		setValue(vArgList[1].c_str());
		Msg(gcString("  New Value: {0}\n", m_szData));
			
		if (m_iFlags & CFLAG_SAVEONEXIT)
			Msg("  Note: Restart needed be for value is applied.\n");

		Msg("\n");
	}
	else
	{
		Msg(gcString("CVAR: {0}\n", m_szName));
		Msg(gcString("  Value: {0}\n", m_szData));	
		Msg("\n");
	}
}

void CVar::setUserData(void* data)
{
	m_pUserData = data;
}

void* CVar::getUserData()
{
	return m_pUserData;
}
