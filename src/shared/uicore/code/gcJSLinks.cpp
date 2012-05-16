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
#include "gcJSLinks.h"

#include "MainApp.h"

REGISTER_JSEXTENDER(DesuraJSLinks);

DesuraJSLinks::DesuraJSLinks() : DesuraJSBase<DesuraJSLinks>("links", "native_binding_links.js")
{
	REG_SIMPLE_JS_VOIDFUNCTION( switchTab, DesuraJSLinks );
	REG_SIMPLE_JS_VOIDFUNCTION( loadUrl, DesuraJSLinks );

	REGISTER_JS_FUNCTION( internalLink, DesuraJSLinks );
}



void DesuraJSLinks::switchTab(int32 tabId)
{
	if (tabId == -1 || tabId >= END_PAGE)
		return;

	g_pMainApp->showPage((PAGE)tabId);
}

void DesuraJSLinks::loadUrl(int32 tabId, gcString url)
{
	if (tabId == -1)
		return;
	
	if (tabId >= END_PAGE || tabId != ITEMS)
		return;
	
	if (url.size() == 0)
		return;

	g_pMainApp->loadUrl(url.c_str(), (PAGE)tabId);
}

JSObjHandle DesuraJSLinks::internalLink(ChromiumDLL::JavaScriptFactoryI *factory, ChromiumDLL::JavaScriptContextI* context, JSObjHandle object, std::vector<JSObjHandle> &args)
{
	if (args.size() < 2)
		return factory->CreateUndefined();

	gcString szId;
	int32 action =0;

	FromJSObject(szId, args[0]);
	FromJSObject(action, args[1]);

	DesuraId id(UTIL::MISC::atoll(szId.c_str()));
	std::vector<std::string> argList;

	for (size_t x=2; x<args.size(); x++)
	{
		gcString extra;

		try
		{
			FromJSObject(extra, args[x]);
			argList.push_back(extra);
		}
		catch (gcException)
		{
		}
	}

	if (action != -1 && action < ACTION_LAST)
		g_pMainApp->handleInternalLink(id, action, argList);

	return factory->CreateUndefined();
}

