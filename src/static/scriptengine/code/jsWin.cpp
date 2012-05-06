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
#include "jsWin.h"


OsJSBinding::OsJSBinding() : DesuraJSBase<OsJSBinding>("os", "installer_binding_os.js")
{
	REG_SIMPLE_JS_VOIDFUNCTION( SetRegistryKey, OsJSBinding);
	REG_SIMPLE_JS_VOIDFUNCTION( DelRegistryKey, OsJSBinding);
	REG_SIMPLE_JS_FUNCTION( GetRegistryKey, OsJSBinding);
	REG_SIMPLE_JS_FUNCTION( IsLinux, OsJSBinding );
	REG_SIMPLE_JS_FUNCTION( IsWindows, OsJSBinding );

	REG_SIMPLE_JS_FUNCTION( SetFirewallAllow, OsJSBinding );
	REG_SIMPLE_JS_VOIDFUNCTION( DelFirewallAllow, OsJSBinding );
	REG_SIMPLE_JS_VOIDFUNCTION( SetCompatiblityMode, OsJSBinding );
}

OsJSBinding::~OsJSBinding()
{
}

void OsJSBinding::SetRegistryKey(gcString key, gcString data)
{
#ifdef WIN32
	UTIL::WIN::setRegValue(key, data);
#else
	throw gcException(ERR_INVALID, "OS is not windows");
#endif
}

void OsJSBinding::DelRegistryKey(gcString key)
{
#ifdef WIN32
	UTIL::WIN::delRegValue(key);
#else
	throw gcException(ERR_INVALID, "OS is not windows");
#endif
}

gcString OsJSBinding::GetRegistryKey(gcString key)
{
#ifdef WIN32
	return UTIL::WIN::getRegValue(key);
#else
	throw gcException(ERR_INVALID, "OS is not windows");
#endif
}

bool OsJSBinding::IsLinux()
{
#ifdef WIN32
	return false;
#else
	return true;
#endif
}

bool OsJSBinding::IsWindows()
{
#ifdef WIN32
	return true;
#else
	return false;
#endif
}

bool OsJSBinding::SetFirewallAllow(gcString exePath, gcString name)
{
#ifdef WIN32
	return UTIL::WIN::setFirewallAllow(exePath.c_str(), name.c_str());
#else
	throw gcException(ERR_INVALID, "OS is not windows");
#endif
}

void OsJSBinding::DelFirewallAllow(gcString exePath)
{
#ifdef WIN32
	return UTIL::WIN::delFirewallAllow(exePath.c_str());
#else
	throw gcException(ERR_INVALID, "OS is not windows");
#endif
}

void OsJSBinding::SetCompatiblityMode(gcString exePath, int32 os, int32 flags)
{
#ifdef WIN32
	gcString key("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers\\{0}", UTIL::STRING::urlEncode(exePath));
	gcString val;

	if (os != -1)
	{
		switch (os)
		{
		case 0:
			val += " WIN95";
			break;

		case 1:
			val += " WIN98";
			break;

		case 2:
			val += " NT4SP5";
			break;

		case 3:
			val += " WIN2000";
			break;
		}
	}

	if (flags != -1)
	{
		if (flags & 1<<1)
			val += " 256COLOR";

		if (flags & 1<<2)
			val += " 640X480";

		if (flags & 1<<3)
			val += " DISABLETHEMES";

		if (flags & 1<<4)
			val += " DISABLEDWM";

		if (flags & 1<<5)
			val += " RUNASADMIN";

		if (flags & 1<<6)
			val += " HIGHDPIAWARE";
	}

	if (val.size() == 0)
	{
		UTIL::WIN::delRegValue(key, true);
	}
	else
	{
		//remove leading space
		val.erase(val.begin());
		UTIL::WIN::setRegValue(key, val, false, true);
	}

#else
	throw gcException(ERR_INVALID, "OS is not windows");
#endif
}
