/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>

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


#ifndef DESURA_JSITEM_H
#define DESURA_JSITEM_H
#ifdef _WIN32
#pragma once
#endif

#include "gcJSBase.h"

class ScriptCoreItemI;

class ItemJSBinding : public DesuraJSBase<ItemJSBinding>
{
public:
	ItemJSBinding();
	~ItemJSBinding();

	virtual const char* getRegistrationCode();
	virtual JSObjHandle execute(ChromiumDLL::JavaScriptFunctionArgs* args);

	static bool addExtender(ChromiumDLL::JavaScriptExtenderI* extender);
	static uint32 AddItem(ScriptCoreItemI* item);
	static void RemoveItem(uint32 itemId);
	static ScriptCoreItemI* GetItem(uint32 itemId);

protected:
	void* GetRealItem(int32 id);
};

#endif //DESURA_JSITEM_H
