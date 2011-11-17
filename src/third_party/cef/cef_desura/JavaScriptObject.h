///////////// Copyright © 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : desura_libcef_dll_wrapper
//   File        : JavaScriptObject.h
//   Description :
//      [TODO: Write the purpose of JavaScriptObject.h.]
//
//   Created On: 5/28/2010 2:00:56 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef DESURA_JAVASCRIPTOBJECT_H
#define DESURA_JAVASCRIPTOBJECT_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
#include "include/cef.h"

class V8ValueBaseWrapper : public CefRefCountWrapper<CefBase>
{
public:
	V8ValueBaseWrapper(CefRefPtr<CefV8Value> object)
	{
		m_pObject = object;
	}

	CefRefPtr<CefV8Value> m_pObject;
};


class JavaScriptObject : public ChromiumDLL::JavaScriptObjectI
{
public:
	JavaScriptObject();
	JavaScriptObject(CefRefPtr<CefV8Value> obj);
	~JavaScriptObject();

	virtual void destory();
	virtual ChromiumDLL::JavaScriptObjectI* clone();


	virtual bool isUndefined();
	virtual bool isNull();
	virtual bool isBool();
	virtual bool isInt();
	virtual bool isDouble();
	virtual bool isString();
	virtual bool isObject();
	virtual bool isArray();
	virtual bool isFunction();
	virtual bool isException();

	virtual bool getBoolValue();
	virtual int getIntValue();
	virtual double getDoubleValue();
	virtual int getStringValue(char* buff, size_t buffsize);

	virtual bool hasValue(const char* key);
	virtual bool hasValue(int index);

	virtual bool deleteValue(const char* key);
	virtual bool deleteValue(int index);

	virtual ChromiumDLL::JSObjHandle getValue(const char* key);
	virtual ChromiumDLL::JSObjHandle getValue(int index);

	virtual bool setValue(const char* key, ChromiumDLL::JSObjHandle value);
	virtual bool setValue(int index, ChromiumDLL::JSObjHandle value);

	virtual int getNumberOfKeys();
	virtual void getKey(int index, char* buff, size_t buffsize);

	virtual int getArrayLength();
	virtual void getFunctionName(char* buff, size_t buffsize);

	virtual ChromiumDLL::JavaScriptExtenderI* getFunctionHandler();
	virtual ChromiumDLL::JSObjHandle executeFunction(ChromiumDLL::JavaScriptFunctionArgs *args);

	virtual void* getUserObject();

	virtual void addRef();
	virtual void delRef();

	CefRefPtr<CefV8Value> getCefV8();
	CefRefPtr<CefBase> getCefBase();

	void setException();

private:
	int m_iRefCount;
	bool m_bIsException;
	CefRefPtr<CefV8Value> m_pObject;
};





#endif //DESURA_JAVASCRIPTOBJECT_H
