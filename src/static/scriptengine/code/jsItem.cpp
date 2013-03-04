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
#include "jsItem.h"
#include "util_thread/BaseThread.h"
#include "ScriptCoreI.h"



ItemJSBinding::ItemJSBinding() : DesuraJSBase<ItemJSBinding>("item", "installer_binding_item.js")
{
	REG_SIMPLE_JS_FUNCTION( GetRealItem, ItemJSBinding );
}

ItemJSBinding::~ItemJSBinding()
{
}

void* ItemJSBinding::GetRealItem(int32 id)
{
	return GetItem((uint32)id);
}


bool AddItemExtender(ChromiumDLL::JavaScriptExtenderI* extender)
{
	return ItemJSBinding::addExtender(extender);
}


std::map<uint32, ScriptCoreItemI*> g_ItemMap;
::Thread::Mutex g_ItemLock;

std::vector<ChromiumDLL::JavaScriptExtenderI*> g_vExtenderList;


uint32 ItemJSBinding::AddItem(ScriptCoreItemI* item)
{
	uint32 hash = UTIL::MISC::RSHash_CSTR(gcString("{0}", (uint64)item));

	g_ItemLock.lock();
	g_ItemMap[hash] = item;
	g_ItemLock.unlock();

	return hash;
}

void ItemJSBinding::RemoveItem(uint32 itemId)
{
	g_ItemLock.lock();

	auto it = g_ItemMap.find(itemId);

	if (it != g_ItemMap.end())
		g_ItemMap.erase(it);

	g_ItemLock.unlock();
}

ScriptCoreItemI* ItemJSBinding::GetItem(uint32 itemId)
{
	ScriptCoreItemI* ret = NULL;

	g_ItemLock.lock();

	auto it = g_ItemMap.find(itemId);

	if (it != g_ItemMap.end())
		ret= it->second;

	g_ItemLock.unlock();

	return ret;
}

bool ItemJSBinding::addExtender(ChromiumDLL::JavaScriptExtenderI* extender)
{
	if (!extender)
		return false;

	bool found = false;

	g_ItemLock.lock();

	for (size_t x=0; x<g_vExtenderList.size(); x++)
	{
		if (g_vExtenderList[x] == extender)
		{
			found = true;
			break;
		}
	}

	if (!found)
		g_vExtenderList.push_back(extender);

	g_ItemLock.unlock();

	return !found;
}

JSObjHandle ItemJSBinding::execute(ChromiumDLL::JavaScriptFunctionArgs* args)
{
	JSObjHandle ret =  DesuraJSBase<ItemJSBinding>::execute(args);
	
	if (!ret->isException() || g_vExtenderList.empty())
		return ret;

	g_ItemLock.lock();

	for (size_t x=0; x<g_vExtenderList.size(); x++)
	{
		ret = g_vExtenderList[x]->execute(args);

		if (!ret->isException())
			break;
	}

	g_ItemLock.unlock();
	return ret;
}

const char* ItemJSBinding::getRegistrationCode()
{
	DesuraJSBase<ItemJSBinding>::getRegistrationCode();

	g_ItemLock.lock();

	for (size_t x=0; x<g_vExtenderList.size(); x++)
	{
		const char* code = g_vExtenderList[x]->getRegistrationCode();
		m_szRegCode.append(code);
	}

	g_ItemLock.unlock();

	return m_szRegCode.c_str();
}
