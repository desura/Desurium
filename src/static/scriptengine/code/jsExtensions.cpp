/*
Desura is the leading indie game distribution platform
Copyright (C) Desura Ltd. <support@desura.com>
          (C) Karol Herbst <git@karolherbst.de>

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
#include "cef_desura_includes/ChromiumBrowserI.h"
#include "v8.h"

#include <vector>

class JSObject : public ChromiumDLL::JavaScriptObjectI
{
public:
	JSObject(v8::Handle<v8::Value> obj, bool isException = false)
	:	m_v8Object(obj)
	{
		m_iRefCount = 1;
		m_bIsException = isException;
	}

	virtual ChromiumDLL::JavaScriptObjectI* clone()
	{
		return new JSObject(m_v8Object);
	}

	virtual bool isUndefined()
	{
		return m_v8Object->IsUndefined();
	}

	virtual bool isNull()
	{
		return m_v8Object->IsNull();
	}

	virtual bool isBool()
	{
		return m_v8Object->IsBoolean();
	}

	virtual bool isInt()
	{
		return m_v8Object->IsInt32();
	}

	virtual bool isDouble()
	{
		return m_v8Object->IsNumber();
	}

	virtual bool isString()
	{
		return m_v8Object->IsString();
	}

	virtual bool isObject()
	{
		return m_v8Object->IsObject();
	}

	virtual bool isArray()
	{
		return m_v8Object->IsArray();
	}

	virtual bool isFunction()
	{
		return m_v8Object->IsFunction();
	}

	virtual bool isException()
	{
		return m_bIsException;
	}

	virtual bool getBoolValue()
	{
		return m_v8Object->BooleanValue();
	}

	virtual int getIntValue()
	{
		return m_v8Object->Int32Value();
	}

	virtual double getDoubleValue()
	{
		return m_v8Object->NumberValue();
	}

	virtual int getStringValue(char* buff, size_t buffsize)
	{
		v8::String::AsciiValue ascii(m_v8Object->ToString());

		if (!buff)
			return strlen(*ascii);

		return Safe::strcpy(buff, buffsize, *ascii);
	}

	virtual bool hasValue(const char* key)
	{
		return false;
	}

	virtual bool hasValue(int index)
	{
		return false;
	}

	virtual bool deleteValue(const char* key)
	{
		return false;
	}

	virtual bool deleteValue(int index)
	{
		return false;
	}

	virtual ChromiumDLL::JSObjHandle getValue(const char* key)
	{
		return NULL;
	}

	virtual ChromiumDLL::JSObjHandle getValue(int index)
	{
		return NULL;
	}

	virtual bool setValue(const char* key, ChromiumDLL::JSObjHandle value)
	{
		return false;
	}

	virtual bool setValue(int index, ChromiumDLL::JSObjHandle value)
	{
		return false;
	}

	virtual int getNumberOfKeys()
	{
		return 0;
	}

	virtual void getKey(int index, char* buff, size_t buffsize)
	{
	}

	virtual int getArrayLength()
	{
		v8::HandleScope handle_scope;
		v8::Local<v8::Object> obj = m_v8Object->ToObject();
		v8::Local<v8::Array> arr = v8::Local<v8::Array>::Cast(obj);
		return arr->Length();
	}

	virtual void getFunctionName(char* buff, size_t buffsize)
	{
	}

	virtual ChromiumDLL::JavaScriptExtenderI* getFunctionHandler()
	{
		return NULL;
	}

	virtual ChromiumDLL::JSObjHandle executeFunction(ChromiumDLL::JavaScriptFunctionArgs* args)
	{
		return NULL;
	}

	void addRef()
	{
		m_iRefCount++;
	}

	void delRef()
	{
		m_iRefCount--;

		if (!m_iRefCount)
			delete this;
	}

	void destory()
	{
		delete this;
	}

	virtual void* getUserObject()
	{
		v8::HandleScope handle_scope;

		if (m_v8Object->IsObject() == false)
			return NULL;

		v8::Local<v8::Object> obj = m_v8Object->ToObject();
		v8::Local<v8::String> key = v8::String::New("ScriptCore::UserData");

		if(obj->Has(key))
			return v8::External::Cast(*(obj->Get(key)))->Value();

		return NULL;
	}


	template <typename T>
	T* getUserObject()
	{
		return (T*)getUserObject();
	}

	v8::Handle<v8::Value> getNative()
	{
		return m_v8Object;
	}

private:
	bool m_bIsException;
	uint32 m_iRefCount;
	v8::Handle<v8::Value> m_v8Object;
};



class JSFactory : public ChromiumDLL::JavaScriptFactoryI
{
public:
	virtual ChromiumDLL::JSObjHandle CreateUndefined()
	{
		v8::HandleScope handle_scope;
		return new JSObject(v8::Undefined());
	}

	virtual ChromiumDLL::JSObjHandle CreateNull()
	{
		v8::HandleScope handle_scope;
		return new JSObject(v8::Null());
	}

	virtual ChromiumDLL::JSObjHandle CreateBool(bool value)
	{
		v8::HandleScope handle_scope;
		return new JSObject(v8::Boolean::New(value));
	}

	virtual ChromiumDLL::JSObjHandle CreateInt(int value)
	{
		v8::HandleScope handle_scope;
		return new JSObject(v8::Int32::New(value));
	}

	virtual ChromiumDLL::JSObjHandle CreateDouble(double value)
	{
		v8::HandleScope handle_scope;
		return new JSObject(v8::Number::New(value));
	}

	virtual ChromiumDLL::JSObjHandle CreateString(const char* value)
	{
		v8::HandleScope handle_scope;

		if (!value)
			return new JSObject(v8::String::New(""));

		return new JSObject(v8::String::New(value));
	}

	virtual ChromiumDLL::JSObjHandle CreateObject()
	{
		v8::HandleScope handle_scope;
		return new JSObject(v8::Object::New());
	}

	virtual ChromiumDLL::JSObjHandle CreateObject(void* userData)
	{
		v8::HandleScope handle_scope;
		v8::Local<v8::Object> obj = v8::Object::New();
		v8::Local<v8::Value> data = v8::External::New(userData);

		obj->Set(v8::String::New("ScriptCore::UserData"), data);

		return new JSObject(obj);
	}

	virtual ChromiumDLL::JSObjHandle CreateArray()
	{
		v8::HandleScope handle_scope;
		return new JSObject(v8::Array::New());
	}

	virtual ChromiumDLL::JSObjHandle CreateFunction(const char* name, ChromiumDLL::JavaScriptExtenderI* handler)
	{
		v8::HandleScope handle_scope;
		return NULL;
	}

	virtual ChromiumDLL::JSObjHandle CreateException(const char* value)
	{
		v8::HandleScope handle_scope;

		if (!value)
			return new JSObject(v8::String::New(""), true);

		return new JSObject(v8::String::New(value), true);
	}
};

JSFactory g_Factory;

class JSExtension : public v8::Extension
{
public:
	JSExtension(ChromiumDLL::JavaScriptExtenderI* extender) : v8::Extension(extender->getName(), extender->getRegistrationCode())
	{
		m_pExtender = extender;
	}

	virtual v8::Handle<v8::FunctionTemplate> GetNativeFunction(v8::Handle<v8::String> name)
	{
		v8::String::AsciiValue ascii(name);
		return v8::FunctionTemplate::New(FunctionCallbackImpl, v8::External::New(this));
	}

	static v8::Handle<v8::Value> FunctionCallbackImpl(const v8::Arguments& args)
	{
		v8::HandleScope handle_scope;
		JSExtension* handler = static_cast<JSExtension*>(v8::External::Cast(*(args.Data()))->Value());

		ChromiumDLL::JSObjHandle* argv = new ChromiumDLL::JSObjHandle[args.Length()];

		for(int i = 0; i < args.Length(); i++)
			argv[i] = new JSObject(args[i]);

		ChromiumDLL::JSObjHandle  obj(new JSObject(args.This()));
		v8::Handle<v8::Value> value = v8::Null();

		v8::Local<v8::Function> funct = args.Callee();
		v8::Handle<v8::Value> name = funct->GetName();
		v8::String::AsciiValue str(name);

		try
		{
			ChromiumDLL::JavaScriptFunctionArgs jsargs;
			jsargs.function = *str;
			jsargs.factory = &g_Factory;
			jsargs.argc = args.Length();
			jsargs.object = obj;
			jsargs.argv = argv;
			jsargs.context = NULL;

			ChromiumDLL::JSObjHandle ret = handler->m_pExtender->execute(&jsargs);
			delete [] argv;

			JSObject *native = dynamic_cast<JSObject*>(ret.get());

			if (native)
				value = native->getNative();

			ret->delRef();
		}
		catch (std::exception& e)
		{
			delete [] argv;
			value = v8::ThrowException(v8::String::New(e.what()));
		}

		return value;
	}

protected:

private:
	ChromiumDLL::JavaScriptExtenderI* m_pExtender;
};


std::vector<ChromiumDLL::JavaScriptExtenderI*> *g_vJSExtenderList=NULL;
v8::ExtensionConfiguration* g_pExtenderConfig = NULL;


void RegisterJSExtender(ChromiumDLL::JavaScriptExtenderI* scheme)
{
	if (g_pExtenderConfig)
		return;

	if (!g_vJSExtenderList)
		g_vJSExtenderList = new std::vector<ChromiumDLL::JavaScriptExtenderI*>();

	g_vJSExtenderList->push_back(scheme);
}

v8::ExtensionConfiguration* RegisterJSBindings()
{
	if (!g_pExtenderConfig)
	{
		std::vector<ChromiumDLL::JavaScriptExtenderI*> &list = *g_vJSExtenderList;

		const char** nameList = new const char*[list.size()];

		for (size_t x=0; x<list.size(); x++)
		{
			v8::DeclareExtension de(new JSExtension(list[x]));
			nameList[x] = list[x]->getName();
		}

		g_pExtenderConfig = new v8::ExtensionConfiguration(list.size(), nameList);
	}

	return g_pExtenderConfig; 
}
