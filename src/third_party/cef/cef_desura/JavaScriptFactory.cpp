///////////// Copyright © 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : desura_libcef_dll_wrapper
//   File        : JavaScriptFactory.cpp
//   Description :
//      [TODO: Write the purpose of JavaScriptFactory.cpp.]
//
//   Created On: 5/28/2010 2:01:05 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#include "JavaScriptFactory.h"
#include "include/cef.h"

#include "JavaScriptExtender.h"
#include "JavaScriptObject.h"

JavaScriptFactory* g_pJavaScriptFactory = NULL;

ChromiumDLL::JavaScriptFactoryI* GetJSFactory()
{
	if (!g_pJavaScriptFactory)
		g_pJavaScriptFactory = new JavaScriptFactory();

	return g_pJavaScriptFactory;
}

class AutoDelete
{
public:
	~AutoDelete()
	{
		if (g_pJavaScriptFactory)
		{
			delete g_pJavaScriptFactory;
			g_pJavaScriptFactory = NULL;
		}
	}
};

AutoDelete ad;



JavaScriptFactory::JavaScriptFactory()
{
}

JavaScriptFactory::~JavaScriptFactory()
{
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateUndefined()
{
	return new JavaScriptObject(CefV8Value::CreateUndefined());
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateNull()
{
	return new JavaScriptObject(CefV8Value::CreateNull());
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateBool(bool value)
{
	return new JavaScriptObject(CefV8Value::CreateBool(value));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateInt(int value)
{
	return new JavaScriptObject(CefV8Value::CreateInt(value));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateDouble(double value)
{
	return new JavaScriptObject(CefV8Value::CreateDouble(value));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateString(const char* value)
{
	return new JavaScriptObject(CefV8Value::CreateString(value));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateArray()
{
	return new JavaScriptObject(CefV8Value::CreateArray());
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateObject()
{
	return new JavaScriptObject(CefV8Value::CreateObject(NULL));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateObject(void* userData)
{
	CefBase* base = new ObjectWrapper(userData);
	return new JavaScriptObject(CefV8Value::CreateObject(CefRefPtr<CefBase>(base)));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateFunction(const char* name, ChromiumDLL::JavaScriptExtenderI* handler)
{
	CefRefPtr<CefV8Handler> e = new JavaScriptExtender(handler);
	return new JavaScriptObject(CefV8Value::CreateFunction(name, e));
}

ChromiumDLL::JSObjHandle JavaScriptFactory::CreateException(const char* value)
{
	JavaScriptObject *ret = new JavaScriptObject(CefV8Value::CreateString(value));
	ret->setException();

	return ret;
}
