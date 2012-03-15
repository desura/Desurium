///////////// Copyright © 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : Miscellaneous Files
//   File        : JavaScriptContext.cpp
//   Description :
//      [TODO: Write the purpose of JavaScriptContext.cpp.]
//
//   Created On: 6/29/2011 11:48:35 AM
//   Created By:  <mailto:>
////////////////////////////////////////////////////////////////////////////

#include "JavaScriptContext.h"
#include "JavaScriptFactory.h"
#include "JavaScriptObject.h"

JavaScriptContext::JavaScriptContext()
{
	m_pContext = CefV8Context::GetCurrentContext();
	m_uiCount = 0;
}

JavaScriptContext::JavaScriptContext(CefRefPtr<CefV8Context> context)
{
	m_pContext = context;
	m_uiCount = 0;
}

void JavaScriptContext::destroy()
{
	delete this;
}

ChromiumDLL::JavaScriptContextI* JavaScriptContext::clone()
{
	return new JavaScriptContext(m_pContext);
}

void JavaScriptContext::enter()
{
	if (m_pContext.get())
	{
		m_pContext->Enter();
		m_uiCount++;
	}
}

void JavaScriptContext::exit()
{
	if (m_pContext.get())
	{
		m_pContext->Exit();
		m_uiCount--;
	}
}

ChromiumDLL::JavaScriptFactoryI* JavaScriptContext::getFactory()
{
	if (!m_pContext.get() || m_uiCount == 0)
		return NULL;

	return GetJSFactory();
}

CefRefPtr<CefV8Context> JavaScriptContext::getCefV8()
{
	return m_pContext;
}

ChromiumDLL::JSObjHandle JavaScriptContext::getGlobalObject()
{
	if (m_pContext.get())
		return new JavaScriptObject(m_pContext->GetGlobal());

	return NULL;
}
