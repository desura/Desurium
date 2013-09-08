/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
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


#ifndef DESURA_SCRIPTCOREINTERNAL_H
#define DESURA_SCRIPTCOREINTERNAL_H
#ifdef _WIN32
#pragma once
#endif

#include "v8.h"

class ScriptCoreInternal
{
public:
	void init();
	void del();
	
	void runScript(const char* file, const char* buff, uint32 size);
	void runString(const char* string);

protected:
	void doRunScript(v8::Handle<v8::Script> script);
	gcString reportException(v8::TryCatch* try_catch);

private:
	friend bool IsV8Init();
	static bool s_IsInit;
	static bool s_Disabled;

	static void OnFatalError(const char* location, const char* message);

	v8::Handle<v8::Context> m_v8Context;
};




#endif //DESURA_SCRIPTCOREINTERNAL_H
