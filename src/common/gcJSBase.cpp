///////////// Copyright 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : uicore
//   File        : gcJSBase.cpp
//   Description :
//      [TODO: Write the purpose of gcJSBase.cpp.]
//
//   Created On: 6/17/2010 10:53:04 AM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#include "Common.h"
#include "gcJSBase.h"

JSDelegateI::~JSDelegateI()
{
}




gcString getString(JSObjHandle& arg)
{
	int size = arg->getStringValue(NULL, 0);

	if (size == 0)
		return "";

	char *buff = new char[size+1];
	arg->getStringValue(buff, size+1);

	buff[size] = 0;

	gcString ret(buff);
	safe_delete(buff);

	return ret;
}


void FromJSObject(PVoid&, JSObjHandle& arg)
{
}

void FromJSObject(bool& ret, JSObjHandle& arg)
{
	if (arg->isBool())
		ret = arg->getBoolValue();
	else if (arg->isInt())
		ret = arg->getIntValue()?true:false;
	else if (arg->isDouble())
		ret = arg->getDoubleValue()?true:false;
	else if (arg->isString())
	{
		gcString b = getString(arg);
		ret = (b != "false" && b != "0");
	}
	else
	{
		throw gcException(ERR_INVALIDDATA, "Failed to convert javascript arg to bool");
	}
}

void FromJSObject(int32 &ret, JSObjHandle& arg)
{
	if (arg->isInt())
		ret = arg->getIntValue();
	else if (arg->isBool())
		ret = arg->getBoolValue();
	else if (arg->isString())
	{
		gcString b = getString(arg);
		int64 i = UTIL::MISC::atoll(b.c_str());
		ret = (int32)i;
	}
	else
	{
		throw gcException(ERR_INVALIDDATA, "Failed to convert javascript arg to int32");
	}
}

void FromJSObject(double& ret, JSObjHandle& arg)
{
	if (arg->isInt())
		ret = arg->getIntValue();
	else if (arg->isDouble())
		ret = arg->getDoubleValue();
	else
		throw gcException(ERR_INVALIDDATA, "Failed to convert javascript arg to double");
}

void FromJSObject(gcString& ret, JSObjHandle& arg)
{
	if (arg->isString())
	{
		ret = getString(arg);
	}
	else if (arg->isBool())
	{
		ret = gcString("{0}", arg->getBoolValue());
	}
	else if (arg->isInt())
	{
		ret = gcString("{0}", arg->getIntValue());
	}
	else if (arg->isDouble())
	{
		ret = gcString("{0}", arg->getDoubleValue());
	}
	else
	{
		throw gcException(ERR_INVALIDDATA, "Failed to convert javascript arg to string");
	}
}

void FromJSObject(gcWString& ret, JSObjHandle& arg)
{
	gcString str;
	FromJSObject(str, arg);
	ret = str;
}

JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const int32 intVal)
{
	return factory->CreateInt(intVal);
}

JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const bool boolVal)
{
	return factory->CreateBool(boolVal);
}

JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const double doubleVal)
{
	return factory->CreateDouble(doubleVal);
}

JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const gcString &stringVal)
{
	return factory->CreateString(stringVal.c_str());
}

JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const gcWString &stringVal)
{
	return factory->CreateString(gcString(stringVal).c_str());
}

JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const std::map<gcString, MapElementI*> &map)
{
	JSObjHandle obj = factory->CreateObject();

	std::for_each(map.begin(), map.end(), [&obj, factory](std::pair<gcString, MapElementI*> pair){
		obj->setValue(pair.first.c_str(), pair.second->toJSObject(factory));
		pair.second->destory();
	});

	return obj;
}

JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const std::map<gcString, gcString> &map)
{
	JSObjHandle obj = factory->CreateObject();

	std::for_each(map.begin(), map.end(), [&obj, factory](std::pair<gcString, gcString> pair){
		obj->setValue(pair.first.c_str(), ToJSObject(factory, pair.second));
	});

	return obj;
}

JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const std::vector<MapElementI*> &map)
{
	JSObjHandle obj = factory->CreateArray();

	for (size_t x=0; x<map.size(); x++)
	{
		obj->setValue(x, map[x]->toJSObject(factory));
		map[x]->destory();
	}

	return obj;
}

JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const MapElementI* cmap)
{
	MapElementI* map = const_cast<MapElementI*>(cmap);

	JSObjHandle ret = map->toJSObject(factory);
	map->destory();
	return ret;
}

JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const void* cobject)
{
	void* object = const_cast<void*>(cobject);
	return factory->CreateObject(object);
}


////////////////////////////////////////////
////////////////////////////////////////////

DesuraJSBaseNonTemplate::DesuraJSBaseNonTemplate(const char* name, const char* bindingFile)
:	m_szName(L"Desura/{0}", name),
	m_szBindingFile("{2}{1}bindings{1}{0}", bindingFile, DIRS_STR, UTIL::OS::getDataPath()),
	m_pContext(NULL)
{}

DesuraJSBaseNonTemplate::~DesuraJSBaseNonTemplate()
{
	for (size_t x=0; x<m_mDelegateList.size(); x++)
		safe_delete(m_mDelegateList[x]);
}

const char* DesuraJSBaseNonTemplate::getName()
{
	return m_szName.c_str();
}

const char* DesuraJSBaseNonTemplate::getRegistrationCode()
{
	char* buff=NULL;
	uint32 size = 0;

	try
	{
		size = UTIL::FS::readWholeFile(UTIL::FS::PathWithFile(m_szBindingFile), &buff);
		m_szRegCode.assign(buff, buff+size);
	}
	catch (gcException &e)
	{
		Warning(gcString("Failed to read js binding file {0}: {1}", m_szBindingFile, e));
	}

	safe_delete(buff);
	return m_szRegCode.c_str();
}

JSObjHandle DesuraJSBaseNonTemplate::execute(ChromiumDLL::JavaScriptFunctionArgs* args)
{
	m_pContext = args->context;

	try
	{
		if (!args->factory || !args->function)
			return NULL;

		uint32 id = UTIL::MISC::RSHash_CSTR(args->function, strlen(args->function));
		JSDelegateI* delegate = m_mDelegateList[id];

		if (!delegate)
			throw gcException(ERR_INVALID, "Function not found");

		if (!preExecuteValidation(args->function, id, args->object, args->argc, args->argv))
			return NULL;

		return delegate->operator()(args->factory, args->context, args->object, args->argc, args->argv);
	}
	catch(std::exception &e)
	{
		return args->factory->CreateException(e.what());
	}
}

void DesuraJSBaseNonTemplate::registerFunction(const char* name, JSDelegateI *delegate)
{
	uint32 id = UTIL::MISC::RSHash_CSTR(name);
	JSDelegateI* oldDelegate = m_mDelegateList[id];

	if (oldDelegate)
	{
		safe_delete(oldDelegate);
		m_mDelegateList.erase(id);
	}

	m_mDelegateList[id] = delegate;
}
