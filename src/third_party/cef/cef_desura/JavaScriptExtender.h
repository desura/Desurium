///////////// Copyright © 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : desura_libcef_dll_wrapper
//   File        : JavaScriptExtender.h
//   Description :
//      [TODO: Write the purpose of JavaScriptExtender.h.]
//
//   Created On: 5/28/2010 2:01:13 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef DESURA_JAVASCRIPTEXTENDER_H
#define DESURA_JAVASCRIPTEXTENDER_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
#include "include/cef.h"

class V8HandleBaseWrapper : public CefBase
{
public:
	V8HandleBaseWrapper(CefRefPtr<CefV8Handler> object)
	{
		m_pObject = object;
		m_iNumRef = 1;
	}

	virtual int AddRef()
	{
		m_iNumRef++;
		return m_iNumRef;
	}

	virtual int Release()
	{
		m_iNumRef--;

		if (m_iNumRef == 0)
			delete this;

		return m_iNumRef;
	}

	virtual int GetRefCt()
	{
		return m_iNumRef;
	}

	int m_iNumRef;
	CefRefPtr<CefV8Handler> m_pObject;
};


class JavaScriptExtender : public CefRefCountWrapper<CefV8Handler>
{
public:
	static bool Register(ChromiumDLL::JavaScriptExtenderI* jse);

	JavaScriptExtender(ChromiumDLL::JavaScriptExtenderI* jse);
	~JavaScriptExtender();

	virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception);

private:
	ChromiumDLL::JavaScriptExtenderI* m_pJSExtender;
};



class JavaScriptWrapper : public ChromiumDLL::JavaScriptExtenderI
{
public:
	JavaScriptWrapper();
	JavaScriptWrapper(CefRefPtr<CefV8Handler> obj);

	virtual void destroy();
	virtual ChromiumDLL::JavaScriptExtenderI* clone();
	virtual ChromiumDLL::JSObjHandle execute(ChromiumDLL::JavaScriptFunctionArgs *args);

	virtual const char* getName();
	virtual const char* getRegistrationCode();

	CefRefPtr<CefV8Handler> getCefV8Handler();
	CefRefPtr<CefBase> getCefBase();

private:
	CefRefPtr<CefV8Handler> m_pObject;
};

#endif //DESURA_JAVASCRIPTEXTENDER_H
