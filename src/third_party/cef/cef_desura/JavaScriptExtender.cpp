///////////// Copyright © 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : desura_libcef_dll_wrapper
//   File        : JavaScriptExtender.cpp
//   Description :
//      [TODO: Write the purpose of JavaScriptExtender.cpp.]
//
//   Created On: 5/28/2010 12:36:21 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#include "JavaScriptExtender.h"
#include "JavaScriptFactory.h"
#include "JavaScriptObject.h"
#include "JavaScriptContext.h"

bool JavaScriptExtender::Register(ChromiumDLL::JavaScriptExtenderI* jse)
{
	return CefRegisterExtension(jse->getName(), jse->getRegistrationCode(), new JavaScriptExtender(jse));
}

JavaScriptExtender::JavaScriptExtender(ChromiumDLL::JavaScriptExtenderI* jse)
{
	m_pJSExtender = jse;
}

JavaScriptExtender::~JavaScriptExtender()
{
	if (m_pJSExtender)
		m_pJSExtender->destroy();
}

bool JavaScriptExtender::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
{
	if (!m_pJSExtender)
		return false;

	size_t argc = arguments.size();
	ChromiumDLL::JSObjHandle *argv = new ChromiumDLL::JSObjHandle[argc];

	for (size_t x=0; x<argc; x++)
		argv[x] = new JavaScriptObject(arguments[x]);

	ChromiumDLL::JavaScriptFunctionArgs args;

	args.function = name.c_str();
	args.argc = argc;
	args.argv = argv;
	args.object = new JavaScriptObject(object);
	args.factory = GetJSFactory();
	args.context = new JavaScriptContext();

	ChromiumDLL::JSObjHandle jsoRes = m_pJSExtender->execute(&args);

	delete [] argv;
	args.context->destroy();

	if (jsoRes.get() == NULL)
		return false;

	if (jsoRes->isException())
	{
		char except[255] = {0};
		jsoRes->getStringValue(except, 255);

		exception.FromASCII(except);
		return true;
	}

	if (!jsoRes->isUndefined())
	{
		JavaScriptObject* jsoRetProper = (JavaScriptObject*)jsoRes.get();

		if (jsoRetProper)
			retval = jsoRetProper->getCefV8();
	}

	return true;
}




JavaScriptWrapper::JavaScriptWrapper()
{
}

JavaScriptWrapper::JavaScriptWrapper(CefRefPtr<CefV8Handler> obj)
{
	m_pObject = obj;
}


void JavaScriptWrapper::destroy()
{
	delete this;
}

ChromiumDLL::JavaScriptExtenderI* JavaScriptWrapper::clone()
{
	return new JavaScriptWrapper(m_pObject);
}

ChromiumDLL::JSObjHandle JavaScriptWrapper::execute(ChromiumDLL::JavaScriptFunctionArgs *args)
{
	int argc = args->argc;
	const char* function = args->function;

	ChromiumDLL::JSObjHandle *argv = args->argv;
	ChromiumDLL::JavaScriptFactoryI *factory = args->factory;
	ChromiumDLL::JavaScriptObjectI* jso = args->object.get();
	
	CefRefPtr<CefV8Value> object;
	CefRefPtr<CefV8Value> ret;
	CefString exception;
	CefV8ValueList arguments;

	JavaScriptObject* jsoRetProper = (JavaScriptObject*)jso;

	if (jsoRetProper)
		object = jsoRetProper->getCefV8();

	for (int x=0; x<argc; x++)
	{
		JavaScriptObject* jsoProper = (JavaScriptObject*)argv[x].get();

		if (jsoProper)
			arguments.push_back(jsoRetProper->getCefV8());
		else
			arguments.push_back(CefV8Value::CreateUndefined());
	}

	m_pObject->Execute(function, object, arguments, ret, exception);

	std::string e(exception.c_str());

	if (e.size() > 0)
		return factory->CreateException(e.c_str());

	return ChromiumDLL::JSObjHandle(new JavaScriptObject(ret));
}



const char* JavaScriptWrapper::getName()
{
	return NULL;
}

const char* JavaScriptWrapper::getRegistrationCode()
{
	return NULL;
}

CefRefPtr<CefV8Handler> JavaScriptWrapper::getCefV8Handler()
{
	return m_pObject;
}

CefRefPtr<CefBase> JavaScriptWrapper::getCefBase()
{
	CefRefPtr<CefBase> base(new V8HandleBaseWrapper(m_pObject));
	return base;
}















