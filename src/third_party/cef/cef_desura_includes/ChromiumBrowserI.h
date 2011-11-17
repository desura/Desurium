///////////// Copyright © 2010 DesuraNet. All rights reserved. /////////////
//
//   Project     : wxChromiumDll
//   File        : wxChromiumI.h
//   Description :
//      [TODO: Write the purpose of wxChromiumI.h.]
//
//   Created On: 5/25/2010 6:12:05 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef DESURA_WXCHROMIUMI_H
#define DESURA_WXCHROMIUMI_H
#ifdef _WIN32
#pragma once
#endif

#include <stdlib.h>

#if defined(_WIN32)
	#ifdef BUILDING_CEF_SHARED 
		#define DLLINTERFACE __declspec(dllexport)
	#else
		#define DLLINTERFACE __declspec(dllimport)
	#endif
#else
	#define DLLINTERFACE __attribute__ ((visibility("default")))
#endif

namespace ChromiumDLL
{
	enum KeyEventType
	{
		KEYEVENT_RAWKEYDOWN = 0,
		KEYEVENT_KEYDOWN,
		KEYEVENT_KEYUP,
		KEYEVENT_CHAR
	};

	class JSObjHandle;
	class ChromiumBrowserI;
	class JavaScriptExtenderI;
	class JavaScriptFunctionArgs;

	class JavaScriptFactoryI
	{
	public:
		virtual JSObjHandle CreateUndefined()=0;
		virtual JSObjHandle CreateNull()=0;
		virtual JSObjHandle CreateBool(bool value)=0;
		virtual JSObjHandle CreateInt(int value)=0;
		virtual JSObjHandle CreateDouble(double value)=0;
		virtual JSObjHandle CreateString(const char* value)=0;
		virtual JSObjHandle CreateObject()=0;
		virtual JSObjHandle CreateObject(void* userData)=0;
		virtual JSObjHandle CreateException(const char* value)=0;
		virtual JSObjHandle CreateArray()=0;
		virtual JSObjHandle CreateFunction(const char* name, ChromiumDLL::JavaScriptExtenderI* handler)=0;
	};

	class JavaScriptContextI
	{
	public:
		//! Destroy the context
		//!
		virtual void destroy()=0;

		//! Clone the context
		//!
		virtual ChromiumDLL::JavaScriptContextI* clone()=0;

		//! Enter context. Must be called on CEF UI thread
		//!
		virtual void enter()=0;

		//! Exit context. Must be called on CEF UI thread after enter
		//!
		virtual void exit()=0;

		//! only valid after enter has been called
		//!
		virtual ChromiumDLL::JavaScriptFactoryI* getFactory()=0;

		//! Gets the current global object for this context. Only valid after enter has been called
		//!
		virtual JSObjHandle getGlobalObject()=0;
	};

	class JavaScriptObjectI
	{
	public:
		//! Clones this object. Must call destroy once done.
		//!
		//! @return JSObject Clone
		//!
		virtual ChromiumDLL::JavaScriptObjectI* clone()=0;

		//! Undefined JSObject
		//!
		virtual bool isUndefined() =0;

		//! Null JSObject
		//!
		virtual bool isNull() =0;

		//! Bool JSObject
		//!
		virtual bool isBool() =0;

		//! Int JSObject
		//!
		virtual bool isInt() =0;

		//! Double JSObject
		//!
		virtual bool isDouble() =0;

		//! String JSObject
		//!
		virtual bool isString() =0;

		//! Object JSObject
		//!
		virtual bool isObject() =0;

		//! Array JSObject
		//!
		virtual bool isArray() =0;

		//! Function JSObject
		//!
		virtual bool isFunction() =0;

		//! Is this an exception
		//!
		virtual bool isException() =0;

		virtual bool getBoolValue() =0;
		virtual int getIntValue() =0;
		virtual double getDoubleValue() =0;

		//! @return String proper size
		virtual int getStringValue(char* buff, size_t buffsize) =0;

		// OBJECT METHODS - These methods are only available on objects. Arrays and
		// functions are also objects. String- and integer-based keys can be used
		// interchangably with the framework converting between them as necessary.
		// Keys beginning with "Cef::" and "v8::" are reserved by the system.

		// Returns true if the object has a value with the specified identifier.
		virtual bool hasValue(const char* key) =0;
		virtual bool hasValue(int index) =0;

		// Delete the value with the specified identifier.
		virtual bool deleteValue(const char* key) =0;
		virtual bool deleteValue(int index) =0;

		//! Returns the value with the specified identifier. 
		virtual JSObjHandle getValue(const char* key) =0;
		virtual JSObjHandle getValue(int index) =0;

		//! Associate value with the specified identifier.
		virtual bool setValue(const char* key, JSObjHandle value) =0;
		virtual bool setValue(int index, JSObjHandle value) =0;

		// Returns the user data, if any, specified when the object was created.
		//virtual CefRefPtr<CefBase> GetUserData() =0;

		virtual int getNumberOfKeys()=0;
		virtual void getKey(int index, char* buff, size_t buffsize)=0;

		// ARRAY METHODS - These methods are only available on arrays.
		// Returns the number of elements in the array.
		virtual int getArrayLength() =0;


		// FUNCTION METHODS - These methods are only available on functions.
		// Returns the function name.
		virtual void getFunctionName(char* buff, size_t buffsize) =0;

		//! Returns the function handler or NULL if not a CEF-created function. 
		//! Must call destroy once done!
		virtual ChromiumDLL::JavaScriptExtenderI* getFunctionHandler() =0;

		//! executes a function.
		//! Must call destroy once done!
		//! args doesnt use function name, or factory
		virtual JSObjHandle executeFunction(ChromiumDLL::JavaScriptFunctionArgs* args) =0;

		virtual void addRef() =0;
		virtual void delRef() =0;

		virtual void* getUserObject() =0;

		template <typename T>
		T* getUserObject()
		{
			return (T*)getUserObject();
		}
	};

	class JSObjHandle
	{
	public:
		explicit JSObjHandle()
		{
			m_pObj = NULL;
		}

		JSObjHandle(ChromiumDLL::JavaScriptObjectI* obj)
		{
			m_pObj = obj;

			if (m_pObj)
				m_pObj->addRef();
		}

		JSObjHandle(const JSObjHandle& obj)
		{
			m_pObj = obj.m_pObj;
			
			if (m_pObj)
				m_pObj->addRef();
		}

		~JSObjHandle()
		{
			if (m_pObj)
				m_pObj->delRef();
		}

		ChromiumDLL::JavaScriptObjectI* operator ->() const
		{
			return m_pObj;
		}

		operator ChromiumDLL::JavaScriptObjectI*() const 
		{ 
			return m_pObj; 
		}

		JSObjHandle& operator=(const JSObjHandle& r) 
		{
			if (m_pObj)
				m_pObj->delRef();

			m_pObj = r.m_pObj;

			if (m_pObj)
				m_pObj->addRef();

			return *this;
		}

		ChromiumDLL::JavaScriptObjectI* get()
		{
			return m_pObj; 
		}

	private:
		ChromiumDLL::JavaScriptObjectI* m_pObj;
	};

	class JavaScriptFunctionArgs
	{
	public:
		const char* function;
		int argc;

		ChromiumDLL::JSObjHandle object;
		ChromiumDLL::JSObjHandle* argv;	//<< array
		ChromiumDLL::JavaScriptFactoryI* factory;
		ChromiumDLL::JavaScriptContextI* context;
	};


	class JavaScriptExtenderI
	{
	public:
		//! Deletes the object. Should never be called by user code!
		//!
		virtual void destroy()=0;

		//! Clones this Extender. Must call destroy once done.
		//!
		//! @return JSExtender Clone
		//!
		virtual ChromiumDLL::JavaScriptExtenderI* clone()=0;


		//! Called when a javascript function is called
		//! Can chuck std::exception
		//!
		//! @param factory Javascript object factory
		//! @parma function Function name
		//! @param object Javascript object that function was called on. Can be NULL
		//! @param argc Number of args
		//! @param argv Args
		//! @return Null if not handled, JavaScriptObjectI Undefined if no return or a JavaScriptObjectI
		//!
		virtual JSObjHandle execute(ChromiumDLL::JavaScriptFunctionArgs* args)=0;

		//! Gets the name to register the extension. i.e. "v8/test"
		//!
		//! @return Extension name
		//!
		virtual const char* getName()=0;


		// Gets the registration javascript
		// Functions implemented by the handler are prototyped using the
		// keyword 'native'. The calling of a native function is restricted to the scope
		// in which the prototype of the native function is defined.
		//
		// Example JavaScript extension code:
		//
		//   // create the 'example' global object if it doesn't already exist.
		//   if (!example)
		//     example = {};
		//   // create the 'example.test' global object if it doesn't already exist.
		//   if (!example.test)
		//     example.test = {};
		//   (function() {
		//     // Define the function 'example.test.myfunction'.
		//     example.test.myfunction = function() {
		//       // Call CefV8Handler::Execute() with the function name 'MyFunction'
		//       // and no arguments.
		//       native function MyFunction();
		//       return MyFunction();
		//     };
		//     // Define the getter function for parameter 'example.test.myparam'.
		//     example.test.__defineGetter__('myparam', function() {
		//       // Call CefV8Handler::Execute() with the function name 'GetMyParam'
		//       // and no arguments.
		//       native function GetMyParam();
		//       return GetMyParam();
		//     });
		//     // Define the setter function for parameter 'example.test.myparam'.
		//     example.test.__defineSetter__('myparam', function(b) {
		//       // Call CefV8Handler::Execute() with the function name 'SetMyParam'
		//       // and a single argument.
		//       native function SetMyParam();
		//       if(b) SetMyParam(b);
		//     });
		//
		//     // Extension definitions can also contain normal JavaScript variables
		//     // and functions.
		//     var myint = 0;
		//     example.test.increment = function() {
		//       myint += 1;
		//       return myint;
		//     };
		//   })();
		//
		// Example usage in the page:
		//
		//   // Call the function.
		//   example.test.myfunction();
		//   // Set the parameter.
		//   example.test.myparam = value;
		//   // Get the parameter.
		//   value = example.test.myparam;
		//   // Call another function.
		//   example.test.increment();
		//
		virtual const char* getRegistrationCode()=0;
	};

	class CookieI
	{
	public:
		virtual void destroy()=0;

		virtual void SetDomain(const char* domain) =0;
		virtual void SetName(const char* name) =0;
		virtual void SetData(const char* data) =0;
		virtual void SetPath(const char* path) =0;
	};

	class PostElementI
	{
	public:
		//! Deletes the object. Should never be called by user code!
		//!
		virtual void destroy()=0;

		virtual bool isFile()=0;
		virtual bool isBytes()=0;

		virtual void setToEmpty() =0;
		virtual void setToFile(const char* fileName) =0;
		virtual void setToBytes(size_t size, const void* bytes) =0;

		virtual void getFile(char *buff, size_t buffsize) =0;

		virtual size_t getBytesCount() =0;
		virtual size_t getBytes(size_t size, void* bytes) =0;
	};

	class PostDataI
	{
	public:
		//! Deletes the object. Should never be called by user code!
		//!
		virtual void destroy()=0;

		virtual size_t getElementCount() =0;
		virtual PostElementI* getElement(size_t index)=0;

		virtual bool removeElement(PostElementI* element) =0;
		virtual bool addElement(PostElementI* element) =0;

		virtual void removeElements() =0;
	};

	class SchemeRequestI
	{
	public:
		//! Deletes the object. Should never be called by user code!
		//!
		virtual void destroy()=0;

		virtual void getURL(char *buff, size_t buffsize) =0;
		virtual void setURL(const char* url) =0;

		virtual void getMethod(char *buff, size_t buffsize) =0;
		virtual void setMethod(const char* method) =0;

		virtual PostDataI* getPostData() =0;
		virtual void setPostData(PostDataI* postData) =0;

		virtual size_t getHeaderCount()=0;

		virtual void getHeaderItem(size_t index, char *key, size_t keysize, char* data, size_t datasize) =0;
		virtual void setHeaderItem(const char* key, const char* data) =0;

		virtual void set(const char* url, const char* method, PostDataI* postData) =0;
	};

	class SchemeCallbackI
	{
	public:
		virtual void responseReady()=0;
		virtual void dataReady()=0;
		virtual void cancel()=0;
	};

	class SchemeExtenderI
	{
	public:
		virtual void destroy()=0;
		virtual SchemeExtenderI* clone(const char* schemeName)=0;

		virtual const char* getSchemeName()=0;
		virtual const char* getHostName()=0;

		//! Processes the request. Call response ready when ready to reply
		//! Set redirect to true to redirect to another url (read from getRedirectUrl)
		//! 
		virtual bool processRequest(SchemeRequestI* request, bool* redirect)=0;

		//! Called when response is ready
		//!
		virtual size_t getResponseSize()=0;

		//! Return NULL to use default
		virtual const char* getResponseMimeType()=0;

		//! Return NULL to cancel redirect
		virtual const char* getRedirectUrl()=0;

		//! Return false to cancel read
		//! Set readSize to zero and return true to wait for callback
		//! 
		virtual bool read(char* buffer, int size, int* readSize)=0;

		//! Cancel request
		//!
		virtual void cancel()=0;

		//! Called so the class can save an instance to the SchemeCallback
		//!
		//! @param callback
		//!
		virtual void registerCallback(SchemeCallbackI* callback)=0;
	};


	class FunctionArgI
	{
	public:
		virtual void setBool(bool value)=0;
		virtual void setInt(int value)=0;
		virtual void setDouble(double value)=0;
		virtual void setString(const char* value)=0;
		virtual void setNull()=0;
		virtual void setVoid()=0;

		virtual bool getBool()=0;
		virtual int getInt()=0;
		virtual double getDouble()=0;
		virtual void getString(char* buff, size_t buffsize)=0;

		virtual bool isBool()=0;
		virtual bool isInt()=0;
		virtual bool isDouble()=0;
		virtual bool isString()=0;
		virtual bool isNull()=0;
		virtual bool isVoid()=0;
	};


	class FunctionArgsI
	{
	public:
		virtual size_t getCount()=0;
		virtual FunctionArgI* getArg(size_t index)=0;
	};



	class FunctionDelegateI
	{
	public:
		virtual void destroy()=0;
		virtual void operator()(ChromiumDLL::FunctionArgsI* args, ChromiumDLL::FunctionArgI* result)=0;
	};

	template <class T>
	class FunctionDelegate : public FunctionDelegateI
	{
	public:
		typedef void (T::*JSItemFunction)(ChromiumDLL::FunctionArgsI*, ChromiumDLL::FunctionArgI*);

		FunctionDelegate(T *t, JSItemFunction function)
		{
			m_pItem = t;
			m_pFunction = function;
		}

		void operator()(ChromiumDLL::FunctionArgsI* args, ChromiumDLL::FunctionArgI* result)
		{
			return (*m_pItem.*m_pFunction)(args, result);
		}

		virtual void destroy()
		{
			delete this;
		}

	private:
		T* m_pItem;
		JSItemFunction m_pFunction;
	};

	template <class TObj>
	FunctionDelegateI* newFunctionDelegate(TObj* pObj, void (TObj::*function)(ChromiumDLL::FunctionArgsI*, ChromiumDLL::FunctionArgI*))
	{
		return new FunctionDelegate<TObj>(pObj, function);
	}


	class FunctionRegisterI
	{
	public:
		virtual void registerFunction(const char* name, FunctionDelegateI* delegate)=0;
	};

	class ChromiumMenuItemI
	{
	public:
		enum TypeFlags
		{
			MENUITEM_SEPERATOR = 1,
			MENUITEM_OPTION,
			MENUITEM_CHECKABLEOPTION,
			MENUITEM_GROUP,
		};

		virtual int getAction()=0;
		virtual int getType()=0;
		virtual const char* getLabel()=0;

		virtual bool isEnabled()=0;
		virtual bool isChecked()=0;
	};

	class ChromiumMenuInfoI
	{
	public:
		enum TypeFlags
		{
		  // No node is selected
		  MENUTYPE_NONE = 0x0,
		  // The top page is selected
		  MENUTYPE_PAGE = 0x1,
		  // A subframe page is selected
		  MENUTYPE_FRAME = 0x2,
		  // A link is selected
		  MENUTYPE_LINK = 0x4,
		  // An image is selected
		  MENUTYPE_IMAGE = 0x8,
		  // There is a textual or mixed selection that is selected
		  MENUTYPE_SELECTION = 0x10,
		  // An editable element is selected
		  MENUTYPE_EDITABLE = 0x20,
		  // A misspelled word is selected
		  MENUTYPE_MISSPELLED_WORD = 0x40,
		  // A video node is selected
		  MENUTYPE_VIDEO = 0x80,
		  // A video node is selected
		  MENUTYPE_AUDIO = 0x100,
		};

		enum EditFlags
		{
		  MENU_CAN_DO_NONE = 0x0,
		  MENU_CAN_UNDO = 0x1,
		  MENU_CAN_REDO = 0x2,
		  MENU_CAN_CUT = 0x4,
		  MENU_CAN_COPY = 0x8,
		  MENU_CAN_PASTE = 0x10,
		  MENU_CAN_DELETE = 0x20,
		  MENU_CAN_SELECT_ALL = 0x40,
		  MENU_CAN_TRANSLATE = 0x80,
		  MENU_CAN_GO_FORWARD = 0x10000000,
		  MENU_CAN_GO_BACK = 0x20000000,
		};

		virtual TypeFlags getTypeFlags()=0;
		virtual EditFlags getEditFlags()=0;

		virtual void getMousePos(int* x, int* y)=0;

		virtual const char* getLinkUrl()=0;
		virtual const char* getImageUrl()=0;
		virtual const char* getPageUrl()=0;
		virtual const char* getFrameUrl()=0;
		virtual const char* getSelectionText()=0;
		virtual const char* getMisSpelledWord()=0;
		virtual const char* getSecurityInfo()=0;

		virtual int getCustomCount()=0;
		virtual ChromiumMenuItemI* getCustomItem(size_t index)=0;

		virtual int* getHWND()=0;
	};

	class ChromiumBrowserEventI
	{
	public:
		//! Before browser loads a new url. Return false to stop
		//!
		//! @param url Url of new page 
		//! @param isMain Is this loading on the main page or loading in an iframe/resource
		//! @return true to continue, false to stop
		//!
		virtual bool onNavigateUrl(const char* url, bool isMain)=0;

		//! When a page starts loading
		//!
		virtual void onPageLoadStart()=0;

		//! When a page finishes loading
		//!
		virtual void onPageLoadEnd()=0;

		//! Show a javascript Alert Box
		//!
		//! @param msg Message to show
		//! @return True if handled, false to show default
		//!
		virtual bool onJScriptAlert(const char* msg)=0;

		//! Show a javascript Confirm Box
		//!
		//! @param msg Message to show
		//! @param result Result of confirm
		//! @return True if handled, false to show default
		//!
		virtual bool onJScriptConfirm(const char* msg, bool* result)=0;

		//! Show a javascript Alert
		//!
		//! @param msg Message to show
		//! @return True if handled, false to show default
		//!
		virtual bool onJScriptPrompt(const char* msg, const char* defualtVal, bool* handled, char result[255])=0;

		//! When a key is pressed
		//!
		//! @param type is the type of keyboard event.
		//! @param code is the windows scan-code for the event.
		//! @param modifiers is a set of bit-flags describing any pressed modifier keys.
		//! @param isSystemKey is set if Windows considers this a 'system key' message;
		//   (see http://msdn.microsoft.com/en-us/library/ms646286(VS.85).aspx)
		//! @return True if handled, false for default handling
		//!
		virtual bool onKeyEvent(KeyEventType type, int code, int modifiers, bool isSystemKey)=0;


		//Logs a console message
		virtual void onLogConsoleMsg(const char* message, const char* source, int line)=0;

		virtual void launchLink(const char* url)=0;

		//! When a page fails to load you can return a custom error page into buff with max size size
		//! 
		//! @param errorCode Code of error
		//! @param url Url error happened on
		//! @param buff Buff to save response into
		//! @param size Max buffer size
		//! 
		//! @return true if handled, false if not
		//!
		virtual bool onLoadError(const char* errorMsg, const char* url, char* buff, size_t size)=0;


		virtual void HandleWndProc(int message, int wparam, int lparam)=0;
		virtual bool HandlePopupMenu(ChromiumMenuInfoI* menuInfo)=0;


		virtual void HandleJSBinding(JavaScriptObjectI* jsObject, JavaScriptFactoryI* factory)=0;
	};

	class ChromiumBrowserI
	{
	public:
		virtual void destroy()=0;

		virtual void onFocus()=0;

#ifdef _WIN32
		virtual void onPaintBg()=0;
		virtual void onPaint()=0;
		virtual void onResize()=0;
#else
		virtual void onResize(int x, int y, int width, int height)=0;
#endif

		virtual void loadUrl(const char* url)=0;
		virtual void loadString(const char* string)=0;

		virtual void stop()=0;
		virtual void refresh(bool ignoreCache = false)=0;
		virtual void back()=0;
		virtual void forward()=0;

		virtual void zoomIn()=0;
		virtual void zoomOut()=0;
		virtual void zoomNormal()=0;

		virtual void print()=0;
		virtual void viewSource()=0;

		virtual void undo()=0;
		virtual void redo()=0;
		virtual void cut()=0;
		virtual void copy()=0;
		virtual void paste()=0;
		virtual void del()=0;
		virtual void selectall()=0;

		virtual void setEventCallback(ChromiumBrowserEventI* cbe)=0;
		virtual void executeJScript(const char* code, const char* scripturl = 0, int startline = 0)=0;

		virtual void showInspector()=0;
		virtual void hideInspector()=0;
		virtual void inspectElement(int x, int y)=0;

		virtual void scroll(int x, int y, int delta, unsigned int flags)=0;

		virtual int* getBrowserHandle()=0;
		virtual ChromiumDLL::JavaScriptContextI* getJSContext()=0;
	};


	typedef bool (*LogMessageHandlerFn)(int severity, const char* str);

	class CallbackI
	{
	public:
		virtual void destroy()=0;
		virtual void run()=0;
	};
}


#ifndef CEF_IGNORE_FUNCTIONS 
#ifdef WIN32
extern "C"
{
	DLLINTERFACE void CEF_DoMsgLoop();
	DLLINTERFACE bool CEF_Init(bool threaded, const char* cachePath, const char* logPath, const char* userAgent);
	DLLINTERFACE void CEF_Stop();

	DLLINTERFACE bool CEF_RegisterJSExtender(ChromiumDLL::JavaScriptExtenderI* extender);
	DLLINTERFACE bool CEF_RegisterSchemeExtender(ChromiumDLL::SchemeExtenderI* extender);

	DLLINTERFACE void CEF_DeleteCookie(const char* url, const char* name);
	DLLINTERFACE ChromiumDLL::CookieI* CEF_CreateCookie();
	DLLINTERFACE void CEF_SetCookie(const char* ulr, ChromiumDLL::CookieI* cookie);

	// Form handle as HWND
	DLLINTERFACE ChromiumDLL::ChromiumBrowserI* CEF_NewChromiumBrowser(int* formHandle, const char *name,  const char* defaultUrl);

	// Return true to handle msg
	DLLINTERFACE void CEF_SetLogHandler(ChromiumDLL::LogMessageHandlerFn logFn);

	DLLINTERFACE void CEF_PostCallback(ChromiumDLL::CallbackI* callback);
}
#endif // TODO LINUX
#endif

#endif //DESURA_WXCHROMIUMI_H
