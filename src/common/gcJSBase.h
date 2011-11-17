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

#ifndef DESURA_GCJSBASE_H
#define DESURA_GCJSBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "cef_desura_includes/ChromiumBrowserI.h"
#include "Event.h"

typedef ChromiumDLL::JSObjHandle JSObjHandle;

void RegisterJSExtender( ChromiumDLL::JavaScriptExtenderI* jsextender );

template <class T>
class RegisterJSExtenderHelper
{
public:
	RegisterJSExtenderHelper()
	{
		::RegisterJSExtender( new T() );
	}

	void noOp()
	{
	}
};


#define REGISTER_JSEXTENDER( jsExtenderName ) static RegisterJSExtenderHelper< jsExtenderName > g_RJS;
#define REGISTER_JS_FUNCTION( function, class ) { registerFunction( #function , newJSDelegate(this, &class::function) ); }

#define REG_SIMPLE_JS_FUNCTION( function, class){ registerFunction( #function , newJSFunctionDelegate(this, &class::function) ); }
#define REG_SIMPLE_JS_VOIDFUNCTION( function, class){ registerFunction( #function , newJSVoidFunctionDelegate(this, &class::function) ); }

#define REG_SIMPLE_JS_OBJ_FUNCTION( function, class){ registerFunction( #function , newJSObjFunctionDelegate(this, &class::function) ); }
#define REG_SIMPLE_JS_OBJ_VOIDFUNCTION( function, class){ registerFunction( #function , newJSVoidObjFunctionDelegate(this, &class::function) ); }



class JSDelegateI
{
public:
	virtual ~JSDelegateI();
	virtual JSObjHandle operator()(ChromiumDLL::JavaScriptFactoryI *factory, ChromiumDLL::JavaScriptContextI* context, JSObjHandle obj, size_t argc, JSObjHandle* argv)=0;
};

class MapElementI
{
public:
	virtual JSObjHandle toJSObject(ChromiumDLL::JavaScriptFactoryI* factory)=0;
	virtual void destory()=0;
};

template <typename T>
class MapElement : public MapElementI
{
public:
	MapElement(T res)
	{
		t = res;
	}

	virtual JSObjHandle toJSObject(ChromiumDLL::JavaScriptFactoryI* factory)
	{
		return ToJSObject(factory, t);
	}

	virtual void destory()
	{
		delete this;
	}

	T t;
};


class PVoid
{
};

template <typename T>
void FromJSObject(std::map<gcString, T> &map, JSObjHandle& arg)
{
	if (arg->isObject() == false)
		throw gcException();

	for (int x=0; x<arg->getNumberOfKeys(); x++)
	{
		char key[255] = {0};
		arg->getKey(x, key, 255);

		if (!key[0])
			continue;

		T t;
		JSObjHandle objH =  arg->getValue(key);
		FromJSObject(t, objH);
		map[gcString(key)] = t;
	}
}

template <typename T>
void FromJSObject(std::vector<T> &list, JSObjHandle& arg)
{
	if (arg->isArray() == false)
		throw gcException();

	for (size_t x=0; x<(size_t)arg->getArrayLength(); x++)
	{
		T t;
		JSObjHandle objH = arg->getValue(x); 
		FromJSObject(t, objH);
		list.push_back(t);
	}
}


namespace UserCore
{
	namespace Item
	{
		class ItemInfoI;
	}
}

void FromJSObject(UserCore::Item::ItemInfoI* &item, JSObjHandle& arg);
void FromJSObject(PVoid&, JSObjHandle& arg);
void FromJSObject(bool& ret, JSObjHandle& arg);
void FromJSObject(int32 &ret, JSObjHandle& arg);
void FromJSObject(double& ret, JSObjHandle& arg);
void FromJSObject(gcString& ret, JSObjHandle& arg);
void FromJSObject(gcWString& ret, JSObjHandle& arg);
void FromJSObject(std::map<gcString, gcString> &map, JSObjHandle& arg);

inline void FromJSObject(JSObjHandle& ret, JSObjHandle& arg)
{
	ret = arg;
}

JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const MapElementI* map);
JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const void* object);
JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const int32 intVal);
JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const bool boolVal);
JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const double doubleVal);
JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const gcString &stringVal);
JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const gcWString &stringVal);
JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const std::map<gcString, MapElementI*> &map);
JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const std::map<gcString, gcString> &map);
JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const std::vector<MapElementI*> &map);

template <typename T>
JSObjHandle ToJSObject(ChromiumDLL::JavaScriptFactoryI* factory, const std::vector<T> &list)
{
	JSObjHandle arr = factory->CreateArray();

	for (size_t x=0; x<list.size(); x++)
	{
		JSObjHandle jObj = ToJSObject(factory, list[x]);
		arr->setValue(x, jObj);
	}

	return arr;
}

template <typename T>
void setDefaultValue(T* &t)
{
	t = NULL;
}

template <typename T>
void setDefaultValue(T& t)
{
}


template <class TObj, typename R, typename A = PVoid, typename B = PVoid, typename C = PVoid, typename D = PVoid, typename E = PVoid, typename F = PVoid>
class JSDelegateFunction : public JSDelegateI
{
public:
	typedef R (TObj::*TFunct0)();
	typedef R (TObj::*TFunct1)(A);
	typedef R (TObj::*TFunct2)(A, B);
	typedef R (TObj::*TFunct3)(A, B, C);
	typedef R (TObj::*TFunct4)(A, B, C, D);
	typedef R (TObj::*TFunct5)(A, B, C, D, E);
	typedef R (TObj::*TFunct6)(A, B, C, D, E, F);

	JSDelegateFunction(TObj* t, TFunct0 f)
	{
		init();
		m_pObj = t;
		m_pFunct0 = f;
		m_uiNumParams = 0;
	}	

	JSDelegateFunction(TObj* t, TFunct1 f)
	{
		init();
		m_pObj = t;
		m_pFunct1 = f;
		m_uiNumParams = 1;
	}	

	JSDelegateFunction(TObj* t, TFunct2 f)
	{
		init();
		m_pObj = t;
		m_pFunct2 = f;
		m_uiNumParams = 2;
	}

	JSDelegateFunction(TObj* t, TFunct3 f)
	{
		init();
		m_pObj = t;
		m_pFunct3 = f;
		m_uiNumParams = 3;
	}

	JSDelegateFunction(TObj* t, TFunct4 f)
	{
		init();
		m_pObj = t;
		m_pFunct4 = f;
		m_uiNumParams = 4;
	}

	JSDelegateFunction(TObj* t, TFunct5 f)
	{
		init();
		m_pObj = t;
		m_pFunct5 = f;
		m_uiNumParams = 5;
	}

	JSDelegateFunction(TObj* t, TFunct6 f)
	{
		init();
		m_pObj = t;
		m_pFunct6 = f;
		m_uiNumParams = 6;
	}

	void init()
	{
		m_pFunct0 = NULL;
		m_pFunct1 = NULL;
		m_pFunct2 = NULL;
		m_pFunct3 = NULL;
		m_pFunct4 = NULL;
		m_pFunct5 = NULL;
		m_pFunct6 = NULL;
	}

	JSObjHandle operator()(ChromiumDLL::JavaScriptFactoryI *factory, ChromiumDLL::JavaScriptContextI* context, JSObjHandle obj, size_t argc, JSObjHandle* argv)
	{
		if (argc < m_uiNumParams)
			throw gcException(ERR_V8, "Not enough paramaters supplied for javascript function call!");

		JSObjHandle ret(NULL);

		A a;
		B b;
		C c;
		D d;
		E e;
		F f;

		setDefaultValue(a);
		setDefaultValue(b);
		setDefaultValue(c);
		setDefaultValue(d);
		setDefaultValue(e);
		setDefaultValue(f);

		try
		{
			switch (m_uiNumParams)
			{
			case 6: 
				FromJSObject(f, argv[5]);
			case 5: 
				FromJSObject(e, argv[4]);
			case 4: 
				FromJSObject(d, argv[3]);
			case 3: 
				FromJSObject(c, argv[2]);
			case 2: 
				FromJSObject(b, argv[1]);
			case 1: 
				FromJSObject(a, argv[0]);
			};

			R r;

			switch (m_uiNumParams)
			{
			case 0: 
				r = (*m_pObj.*m_pFunct0)(); 
				break;

			case 1: 
				r = (*m_pObj.*m_pFunct1)(a);
				break;

			case 2: 
				r = (*m_pObj.*m_pFunct2)(a, b);
				break;

			case 3: 
				r = (*m_pObj.*m_pFunct3)(a, b, c);
				break;

			case 4: 
				r = (*m_pObj.*m_pFunct4)(a, b, c, d);
				break;

			case 5: 
				r = (*m_pObj.*m_pFunct5)(a, b, c, d, e); 
				break;

			case 6: 
				r = (*m_pObj.*m_pFunct6)(a, b, c, d, e, f);
				break;
			};
			
			ret = ToJSObject(factory, r);
		}
		catch (gcException &e)
		{
			throw gcException(ERR_V8, gcString("Failed to convert javascript argument: {0}", e));
		}

		return ret;
	}

private:
	// pointer to object
	TObj* m_pObj;     

	// pointer to member function
	TFunct0 m_pFunct0;
	TFunct1 m_pFunct1;
	TFunct2 m_pFunct2;  
	TFunct3 m_pFunct3;  
	TFunct4 m_pFunct4;  
	TFunct5 m_pFunct5;  
	TFunct6 m_pFunct6;  
	
	uint32 m_uiNumParams;
};


template <class TObj, typename A = PVoid, typename B = PVoid, typename C = PVoid, typename D = PVoid, typename E = PVoid, typename F = PVoid>
class JSDelegateVoidFunction : public JSDelegateI
{
public:
	typedef void (TObj::*TFunct0)();
	typedef void (TObj::*TFunct1)(A);
	typedef void (TObj::*TFunct2)(A, B);
	typedef void (TObj::*TFunct3)(A, B, C);
	typedef void (TObj::*TFunct4)(A, B, C, D);
	typedef void (TObj::*TFunct5)(A, B, C, D, E);
	typedef void (TObj::*TFunct6)(A, B, C, D, E, F);

	JSDelegateVoidFunction(TObj* t, TFunct0 f)
	{
		init();
		m_pObj = t;
		m_pFunct0 = f;
		m_uiNumParams = 0;
	}	

	JSDelegateVoidFunction(TObj* t, TFunct1 f)
	{
		init();
		m_pObj = t;
		m_pFunct1 = f;
		m_uiNumParams = 1;
	}	

	JSDelegateVoidFunction(TObj* t, TFunct2 f)
	{
		init();
		m_pObj = t;
		m_pFunct2 = f;
		m_uiNumParams = 2;
	}

	JSDelegateVoidFunction(TObj* t, TFunct3 f)
	{
		init();
		m_pObj = t;
		m_pFunct3 = f;
		m_uiNumParams = 3;
	}

	JSDelegateVoidFunction(TObj* t, TFunct4 f)
	{
		init();
		m_pObj = t;
		m_pFunct4 = f;
		m_uiNumParams = 4;
	}

	JSDelegateVoidFunction(TObj* t, TFunct5 f)
	{
		init();
		m_pObj = t;
		m_pFunct5 = f;
		m_uiNumParams = 5;
	}

	JSDelegateVoidFunction(TObj* t, TFunct6 f)
	{
		init();
		m_pObj = t;
		m_pFunct6 = f;
		m_uiNumParams = 6;
	}

	void init()
	{
		m_pFunct0 = NULL;
		m_pFunct1 = NULL;
		m_pFunct2 = NULL;
		m_pFunct3 = NULL;
		m_pFunct4 = NULL;
		m_pFunct5 = NULL;
		m_pFunct6 = NULL;
	}

	JSObjHandle operator()(ChromiumDLL::JavaScriptFactoryI *factory, ChromiumDLL::JavaScriptContextI* context, JSObjHandle obj, size_t argc, JSObjHandle* argv)
	{
		if (argc < m_uiNumParams)
			throw gcException(ERR_V8, "Not enough paramaters supplied for javascript function call!");

		A a;
		B b;
		C c;
		D d;
		E e;
		F f;

		setDefaultValue(a);
		setDefaultValue(b);
		setDefaultValue(c);
		setDefaultValue(d);
		setDefaultValue(e);
		setDefaultValue(f);

		try
		{
			switch (m_uiNumParams)
			{
			case 6: 
				FromJSObject(f, argv[5]);
			case 5: 
				FromJSObject(e, argv[4]);
			case 4: 
				FromJSObject(d, argv[3]);
			case 3: 
				FromJSObject(c, argv[2]);
			case 2: 
				FromJSObject(b, argv[1]);
			case 1:
				FromJSObject(a, argv[0]);
			};

			switch (m_uiNumParams)
			{
			case 0: 
				(*m_pObj.*m_pFunct0)(); 
				break;

			case 1: 
				(*m_pObj.*m_pFunct1)(a); 
				break;

			case 2: 
				(*m_pObj.*m_pFunct2)(a, b); 
				break;

			case 3: 
				(*m_pObj.*m_pFunct3)(a, b, c); 
				break;

			case 4: 
				(*m_pObj.*m_pFunct4)(a, b, c, d); 
				break;

			case 5: 
				(*m_pObj.*m_pFunct5)(a, b, c, d, e); 
				break;

			case 6: 
				(*m_pObj.*m_pFunct6)(a, b, c, d, e, f); 
				break;
			};

		}
		catch (gcException &e)
		{
			throw gcException(ERR_V8, gcString("Failed to convert javascript argument: {0}", e).c_str());
		}

		return factory->CreateUndefined();
	}

private:
	// pointer to object
	TObj* m_pObj;     

	// pointer to member function
	TFunct0 m_pFunct0;
	TFunct1 m_pFunct1;
	TFunct2 m_pFunct2;  
	TFunct3 m_pFunct3;  
	TFunct4 m_pFunct4;  
	TFunct5 m_pFunct5;  
	TFunct6 m_pFunct6;  
	
	uint32 m_uiNumParams;
};




template <class TObj, class O, typename R, typename A = PVoid, typename B = PVoid, typename C = PVoid, typename D = PVoid, typename E = PVoid>
class JSDelegateObjFunction : public JSDelegateI
{
public:
	typedef R (TObj::*TFunct0)(O*);
	typedef R (TObj::*TFunct1)(O*, A);
	typedef R (TObj::*TFunct2)(O*, A, B);
	typedef R (TObj::*TFunct3)(O*, A, B, C);
	typedef R (TObj::*TFunct4)(O*, A, B, C, D);
	typedef R (TObj::*TFunct5)(O*, A, B, C, D, E);

	JSDelegateObjFunction(TObj* t, TFunct0 f)
	{
		init();
		m_pObj = t;
		m_pFunct0 = f;
		m_uiNumParams = 0;
	}	

	JSDelegateObjFunction(TObj* t, TFunct1 f)
	{
		init();
		m_pObj = t;
		m_pFunct1 = f;
		m_uiNumParams = 1;
	}	

	JSDelegateObjFunction(TObj* t, TFunct2 f)
	{
		init();
		m_pObj = t;
		m_pFunct2 = f;
		m_uiNumParams = 2;
	}

	JSDelegateObjFunction(TObj* t, TFunct3 f)
	{
		init();
		m_pObj = t;
		m_pFunct3 = f;
		m_uiNumParams = 3;
	}

	JSDelegateObjFunction(TObj* t, TFunct4 f)
	{
		init();
		m_pObj = t;
		m_pFunct4 = f;
		m_uiNumParams = 4;
	}

	JSDelegateObjFunction(TObj* t, TFunct5 f)
	{
		init();
		m_pObj = t;
		m_pFunct5 = f;
		m_uiNumParams = 5;
	}

	void init()
	{
		m_pFunct0 = NULL;
		m_pFunct1 = NULL;
		m_pFunct2 = NULL;
		m_pFunct3 = NULL;
		m_pFunct4 = NULL;
		m_pFunct5 = NULL;
	}

	JSObjHandle operator()(ChromiumDLL::JavaScriptFactoryI *factory, ChromiumDLL::JavaScriptContextI* context, JSObjHandle obj, size_t argc, JSObjHandle* argv)
	{
		if (argc < m_uiNumParams+1)
			throw gcException(ERR_V8, "Not enough paramaters supplied for javascript function call!");

		JSObjHandle ret(NULL);

		O* o = NULL;
		if (argv[0]->isObject())
			o = argv[0]->getUserObject<O>();

		A a;
		B b;
		C c;
		D d;
		E e;

		setDefaultValue(a);
		setDefaultValue(b);
		setDefaultValue(c);
		setDefaultValue(d);
		setDefaultValue(e);

		try
		{
			switch (m_uiNumParams)
			{
			case 5: 
				FromJSObject(e, argv[5]);
			case 4: 
				FromJSObject(d, argv[4]);
			case 3: 
				FromJSObject(c, argv[3]);
			case 2: 
				FromJSObject(b, argv[2]);
			case 1: 
				FromJSObject(a, argv[1]);
			};

			R r;

			switch (m_uiNumParams)
			{
			case 0: 
				r = (*m_pObj.*m_pFunct0)(o);
				break;

			case 1: 
				r = (*m_pObj.*m_pFunct1)(o, a); 
				break;

			case 2: 
				r = (*m_pObj.*m_pFunct2)(o, a, b); 
				break;

			case 3: 
				r = (*m_pObj.*m_pFunct3)(o, a, b, c); 
				break;

			case 4: 
				r = (*m_pObj.*m_pFunct4)(o, a, b, c, d);
				break;

			case 5: 
				r = (*m_pObj.*m_pFunct5)(o, a, b, c, d, e); 
				break;
			};

			ret = ToJSObject(factory, r); 

		}
		catch (gcException &e)
		{
			throw gcException(ERR_V8, gcString("Failed to convert javascript argument: {0}", e).c_str());
		}

		return ret;
	}

private:
	// pointer to object
	TObj* m_pObj;     

	// pointer to member function
	TFunct0 m_pFunct0;
	TFunct1 m_pFunct1;
	TFunct2 m_pFunct2;  
	TFunct3 m_pFunct3;  
	TFunct4 m_pFunct4;  
	TFunct5 m_pFunct5;  
	
	uint32 m_uiNumParams;
};




template <class TObj, class O, typename A = PVoid, typename B = PVoid, typename C = PVoid, typename D = PVoid, typename E = PVoid>
class JSDelegateVoidObjFunction : public JSDelegateI
{
public:
	typedef void (TObj::*TFunct0)(O*);
	typedef void (TObj::*TFunct1)(O*, A);
	typedef void (TObj::*TFunct2)(O*, A, B);
	typedef void (TObj::*TFunct3)(O*, A, B, C);
	typedef void (TObj::*TFunct4)(O*, A, B, C, D);
	typedef void (TObj::*TFunct5)(O*, A, B, C, D, E);

	JSDelegateVoidObjFunction(TObj* t, TFunct0 f)
	{
		init();
		m_pObj = t;
		m_pFunct0 = f;
		m_uiNumParams = 0;
	}	

	JSDelegateVoidObjFunction(TObj* t, TFunct1 f)
	{
		init();
		m_pObj = t;
		m_pFunct1 = f;
		m_uiNumParams = 1;
	}	

	JSDelegateVoidObjFunction(TObj* t, TFunct2 f)
	{
		init();
		m_pObj = t;
		m_pFunct2 = f;
		m_uiNumParams = 2;
	}

	JSDelegateVoidObjFunction(TObj* t, TFunct3 f)
	{
		init();
		m_pObj = t;
		m_pFunct3 = f;
		m_uiNumParams = 3;
	}

	JSDelegateVoidObjFunction(TObj* t, TFunct4 f)
	{
		init();
		m_pObj = t;
		m_pFunct4 = f;
		m_uiNumParams = 4;
	}

	JSDelegateVoidObjFunction(TObj* t, TFunct5 f)
	{
		init();
		m_pObj = t;
		m_pFunct5 = f;
		m_uiNumParams = 5;
	}


	void init()
	{
		m_pFunct0 = NULL;
		m_pFunct1 = NULL;
		m_pFunct2 = NULL;
		m_pFunct3 = NULL;
		m_pFunct4 = NULL;
		m_pFunct5 = NULL;
	}

	JSObjHandle operator()(ChromiumDLL::JavaScriptFactoryI *factory, ChromiumDLL::JavaScriptContextI* context, JSObjHandle obj, size_t argc, JSObjHandle* argv)
	{
		if (argc < m_uiNumParams+1)
			throw gcException(ERR_V8, "Not enough paramaters supplied for javascript function call!");

		JSObjHandle ret(NULL);

		O* o = NULL;
		if (argv[0]->isObject())
			o = argv[0]->getUserObject<O>();

		A a;
		B b;
		C c;
		D d;
		E e;

		setDefaultValue(a);
		setDefaultValue(b);
		setDefaultValue(c);
		setDefaultValue(d);
		setDefaultValue(e);

		try
		{
			switch (m_uiNumParams)
			{
			case 5: 
				FromJSObject(e, argv[5]);
			case 4: 
				FromJSObject(d, argv[4]);
			case 3: 
				FromJSObject(c, argv[3]);
			case 2: 
				FromJSObject(b, argv[2]);
			case 1: 
				FromJSObject(a, argv[1]);
			};

			switch (m_uiNumParams)
			{
			case 0: 
				(*m_pObj.*m_pFunct0)(o); 
				break;

			case 1: 
				(*m_pObj.*m_pFunct1)(o, a); 
				break;

			case 2: 
				(*m_pObj.*m_pFunct2)(o, a, b); 
				break;

			case 3: 
				(*m_pObj.*m_pFunct3)(o, a, b, c); 
				break;

			case 4: 
				(*m_pObj.*m_pFunct4)(o, a, b, c, d); 
				break;

			case 5: 
				(*m_pObj.*m_pFunct5)(o, a, b, c, d, e); 
				break;
			};

		}
		catch (gcException &e)
		{
			throw gcException(ERR_V8, gcString("Failed to convert javascript argument: {0}", e).c_str());
		}

		return factory->CreateUndefined();
	}

private:
	// pointer to object
	TObj* m_pObj;     

	// pointer to member function
	TFunct0 m_pFunct0;
	TFunct1 m_pFunct1;
	TFunct2 m_pFunct2;  
	TFunct3 m_pFunct3;  
	TFunct4 m_pFunct4;  
	TFunct5 m_pFunct5;  
	
	uint32 m_uiNumParams;
};








template <class T>
class JSDelegate : public JSDelegateI
{
public:
	typedef JSObjHandle (T::*JSItemFunction)(ChromiumDLL::JavaScriptFactoryI*, ChromiumDLL::JavaScriptContextI*, JSObjHandle, std::vector<JSObjHandle> &);

	JSDelegate(T *t, JSItemFunction function)
	{
		m_pItem = t;
		m_pFunction = function;
	}

	JSObjHandle operator()(ChromiumDLL::JavaScriptFactoryI *factory, ChromiumDLL::JavaScriptContextI* context, JSObjHandle obj, size_t argc, JSObjHandle* argv)
	{
		std::vector<JSObjHandle> args;

		for (size_t x=0; x<argc; x++)
			args.push_back(argv[x]);

		return (*m_pItem.*m_pFunction)(factory, context, obj, args);
	}

private:
	T* m_pItem;
	JSItemFunction m_pFunction;
};

template <class TObj>
JSDelegateI* newJSDelegate(TObj* pObj, JSObjHandle (TObj::*function)(ChromiumDLL::JavaScriptFactoryI*, ChromiumDLL::JavaScriptContextI*, JSObjHandle, std::vector<JSObjHandle> &))
{
	return new JSDelegate<TObj>(pObj, function);
}




template <class TObj>
JSDelegateI* newJSVoidFunctionDelegate(TObj* pObj, void (TObj::*func)())
{
	return new JSDelegateVoidFunction<TObj>(pObj, func);
}

template <class TObj, typename A>
JSDelegateI* newJSVoidFunctionDelegate(TObj* pObj, void (TObj::*func)(A))
{
	return new JSDelegateVoidFunction<TObj, A>(pObj, func);
}

template <class TObj, typename A, typename B>
JSDelegateI* newJSVoidFunctionDelegate(TObj* pObj, void (TObj::*func)(A, B))
{
	return new JSDelegateVoidFunction<TObj, A, B>(pObj, func);
}

template <class TObj, typename A, typename B, typename C>
JSDelegateI* newJSVoidFunctionDelegate(TObj* pObj, void (TObj::*func)(A, B, C))
{
	return new JSDelegateVoidFunction<TObj, A, B, C>(pObj, func);
}

template <class TObj, typename A, typename B, typename C, typename D>
JSDelegateI* newJSVoidFunctionDelegate(TObj* pObj, void (TObj::*func)(A, B, C, D))
{
	return new JSDelegateVoidFunction<TObj, A, B, C, D>(pObj, func);
}

template <class TObj, typename A, typename B, typename C, typename D, typename E>
JSDelegateI* newJSVoidFunctionDelegate(TObj* pObj, void (TObj::*func)(A, B, C, D, E))
{
	return new JSDelegateVoidFunction<TObj, A, B, C, D, E>(pObj, func);
}

template <class TObj, typename A, typename B, typename C, typename D, typename E, typename F>
JSDelegateI* newJSVoidFunctionDelegate(TObj* pObj, void (TObj::*func)(A, B, C, D, E, F))
{
	return new JSDelegateVoidFunction<TObj, A, B, C, D, E, F>(pObj, func);
}





template <class TObj, typename R>
JSDelegateI* newJSFunctionDelegate(TObj* pObj, R (TObj::*func)())
{
	return new JSDelegateFunction<TObj, R, PVoid, PVoid, PVoid, PVoid, PVoid, PVoid>(pObj, func);
}

template <class TObj, typename R, typename A>
JSDelegateI* newJSFunctionDelegate(TObj* pObj, R (TObj::*func)(A))
{
	return new JSDelegateFunction<TObj, R, A, PVoid, PVoid, PVoid, PVoid, PVoid>(pObj, func);
}

template <class TObj, typename R, typename A, typename B>
JSDelegateI* newJSFunctionDelegate(TObj* pObj, R (TObj::*func)(A, B))
{
	return new JSDelegateFunction<TObj, R, A, B, PVoid, PVoid, PVoid, PVoid>(pObj, func);
}

template <class TObj, typename R, typename A, typename B, typename C>
JSDelegateI* newJSFunctionDelegate(TObj* pObj, R (TObj::*func)(A, B, C))
{
	return new JSDelegateFunction<TObj, R, A, B, C, PVoid, PVoid, PVoid>(pObj, func);
}

template <class TObj, typename R, typename A, typename B, typename C, typename D>
JSDelegateI* newJSFunctionDelegate(TObj* pObj, R (TObj::*func)(A, B, C, D))
{
	return new JSDelegateFunction<TObj, R, A, B, C, D, PVoid, PVoid>(pObj, func);
}

template <class TObj, typename R, typename A, typename B, typename C, typename D, typename E>
JSDelegateI* newJSFunctionDelegate(TObj* pObj, R (TObj::*func)(A, B, C, D, E))
{
	return new JSDelegateFunction<TObj, R, A, B, C, D, E, PVoid>(pObj, func);
}

template <class TObj, typename R, typename A, typename B, typename C, typename D, typename E, typename F>
JSDelegateI* newJSFunctionDelegate(TObj* pObj, R (TObj::*func)(A, B, C, D, E, F))
{
	return new JSDelegateFunction<TObj, R, A, B, C, D, E, F>(pObj, func);
}





template <class TObj, class O>
JSDelegateI* newJSVoidObjFunctionDelegate(TObj* pObj, void (TObj::*func)(O*))
{
	return new JSDelegateVoidObjFunction<TObj, O>(pObj, func);
}

template <class TObj, class O, typename A>
JSDelegateI* newJSVoidObjFunctionDelegate(TObj* pObj, void (TObj::*func)(O*, A))
{
	return new JSDelegateVoidObjFunction<TObj, O, A>(pObj, func);
}

template <class TObj, class O, typename A, typename B>
JSDelegateI* newJSVoidObjFunctionDelegate(TObj* pObj, void (TObj::*func)(O*, A, B))
{
	return new JSDelegateVoidObjFunction<TObj, O, A, B>(pObj, func);
}

template <class TObj, class O, typename A, typename B, typename C>
JSDelegateI* newJSVoidObjFunctionDelegate(TObj* pObj, void (TObj::*func)(O*, A, B, C))
{
	return new JSDelegateVoidObjFunction<TObj, O, A, B, C>(pObj, func);
}

template <class TObj, class O, typename A, typename B, typename C, typename D>
JSDelegateI* newJSVoidObjFunctionDelegate(TObj* pObj, void (TObj::*func)(O*, A, B, C, D))
{
	return new JSDelegateVoidObjFunction<TObj, O, A, B, C, D>(pObj, func);
}

template <class TObj, class O, typename A, typename B, typename C, typename D, typename E>
JSDelegateI* newJSVoidObjFunctionDelegate(TObj* pObj, void (TObj::*func)(O*, A, B, C, D, E))
{
	return new JSDelegateVoidObjFunction<TObj, O, A, B, C, D, E>(pObj, func);
}


template <class TObj, class O, typename R>
JSDelegateI* newJSObjFunctionDelegate(TObj* pObj, R (TObj::*func)(O*))
{
	return new JSDelegateObjFunction<TObj, O, R>(pObj, func);
}

template <class TObj, class O, typename R, typename A>
JSDelegateI* newJSObjFunctionDelegate(TObj* pObj, R (TObj::*func)(O*, A))
{
	return new JSDelegateObjFunction<TObj, O, R, A>(pObj, func);
}

template <class TObj, class O, typename R, typename A, typename B>
JSDelegateI* newJSObjFunctionDelegate(TObj* pObj, R (TObj::*func)(O*, A, B))
{
	return new JSDelegateObjFunction<TObj, O, R, A, B>(pObj, func);
}

template <class TObj, class O, typename R, typename A, typename B, typename C>
JSDelegateI* newJSObjFunctionDelegate(TObj* pObj, R (TObj::*func)(O*, A, B, C))
{
	return new JSDelegateObjFunction<TObj, O, R, A, B, C>(pObj, func);
}

template <class TObj, class O, typename R, typename A, typename B, typename C, typename D>
JSDelegateI* newJSObjFunctionDelegate(TObj* pObj, R (TObj::*func)(O*, A, B, C, D))
{
	return new JSDelegateObjFunction<TObj, O, R, A, B, C, D>(pObj, func);
}

template <class TObj, class O, typename R, typename A, typename B, typename C, typename D, typename E>
JSDelegateI* newJSObjFunctionDelegate(TObj* pObj, R (TObj::*func)(O*, A, B, C, D, E))
{
	return new JSDelegateObjFunction<TObj, O, R, A, B, C, D, E>(pObj, func);
}

























class DesuraJSBaseNonTemplate : public ChromiumDLL::JavaScriptExtenderI
{
public:
	DesuraJSBaseNonTemplate(const char* name, const char* bindingFile);
	~DesuraJSBaseNonTemplate();

	virtual const char* getName();
	virtual const char* getRegistrationCode();

	JSObjHandle execute(ChromiumDLL::JavaScriptFunctionArgs* args);

	void registerFunction(const char* name, JSDelegateI *delegate);
	virtual bool preExecuteValidation(const char* function, uint32 functionHash, JSObjHandle object, size_t argc, JSObjHandle* argv){return true;}

	virtual ChromiumDLL::JavaScriptContextI* getContext()
	{
		return m_pContext;
	}

protected:
	uint32 find(uint32 hash);
	uint32 find(uint32 hash, uint32 l, uint32 f);

	gcString m_szRegCode;

private:
	gcString m_szBindingFile;
	gcString m_szName;
	std::vector<std::pair<uint32, JSDelegateI*>> m_mDelegateList;
	ChromiumDLL::JavaScriptContextI* m_pContext;
};

template <class T>
class DesuraJSBase : public DesuraJSBaseNonTemplate
{
public:
	DesuraJSBase(const char* name, const char* bindingFile) : DesuraJSBaseNonTemplate(name, bindingFile)
	{
	}

	virtual JavaScriptExtenderI* clone()
	{
		return new T();
	}

	virtual void destroy()
	{
		delete this;
	}
};


#endif //DESURA_GCJSBASE_H
