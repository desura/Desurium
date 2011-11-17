///////////// Copyright © 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : Miscellaneous Files
//   File        : JavaScriptContext.h
//   Description :
//      [TODO: Write the purpose of JavaScriptContext.h.]
//
//   Created On: 6/29/2011 11:48:30 AM
//   Created By:  <mailto:>
////////////////////////////////////////////////////////////////////////////

#ifndef DESURA_JAVASCRIPTCONTEXT_H
#define DESURA_JAVASCRIPTCONTEXT_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
#include "include/cef.h"

class JavaScriptContext : public ChromiumDLL::JavaScriptContextI
{
public:
	JavaScriptContext();
	JavaScriptContext(CefRefPtr<CefV8Context> context);

	virtual void destroy();
	virtual ChromiumDLL::JavaScriptContextI* clone();

	virtual void enter();
	virtual void exit();

	virtual ChromiumDLL::JavaScriptFactoryI* getFactory();
	virtual ChromiumDLL::JSObjHandle getGlobalObject();

	CefRefPtr<CefV8Context> getCefV8();

private:
	uint32 m_uiCount;
	CefRefPtr<CefV8Context> m_pContext;
};




#endif //DESURA_JAVASCRIPTCONTEXT_H
