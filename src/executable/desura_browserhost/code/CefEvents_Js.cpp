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
#include "CefEvents.h"
#include "gcJSBase.h"
#include "IPCBrowser.h"



class CrumbExtender : public DesuraJSBase<CrumbExtender>
{
public:
	CrumbExtender() : DesuraJSBase("CrumbExtender", "host_binding.js")
	{
		REG_SIMPLE_JS_VOIDFUNCTION( clearCrumbs, CrumbExtender );
		REG_SIMPLE_JS_VOIDFUNCTION( addCrumb, CrumbExtender );
		REG_SIMPLE_JS_VOIDFUNCTION( updateCounts, CrumbExtender );
	}

protected:
	void clearCrumbs()
	{
		GetIPCBrowser()->clearCrumbs();
	}

	void addCrumb(gcString name, gcString val)
	{
		GetIPCBrowser()->addCrumb(name.c_str(), val.c_str());
	}

	void updateCounts(int32 msg, int32 updates, int32 threads, int32 cart)
	{
		GetIPCBrowser()->setCounts(msg, updates, threads, cart);
	}
};

REGISTER_JSEXTENDER(CrumbExtender);



class ObjectExtender : public DesuraJSBase<ObjectExtender>
{
public:
	ObjectExtender() : DesuraJSBase("ObjectExtender", "")
	{
	}

	ObjectExtender(const char* name) : DesuraJSBase(name, "")
	{
	}
};


void EventHandler::HandleJSBinding(ChromiumDLL::JavaScriptObjectI* jsObject, ChromiumDLL::JavaScriptFactoryI* factory)
{

}
