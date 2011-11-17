///////////// Copyright © 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : desura_libcef_dll_wrapper
//   File        : Cookie.cpp
//   Description :
//      [TODO: Write the purpose of Cookie.cpp.]
//
//   Created On: 6/7/2010 2:16:43 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#include "ChromiumBrowserI.h"
#include "include/cef.h"

class Cookie : public ChromiumDLL::CookieI
{
public:
	Cookie()
	{
		m_rCookie.secure = false;
		m_rCookie.httponly = false;
	}

	virtual void destroy()
	{
		delete this;
	}

	virtual void SetDomain(const char* domain)
	{
		cef_string_copy(domain, strlen(domain), &m_rCookie.domain);
	}

	virtual void SetName(const char* name)
	{
		cef_string_copy(name, strlen(name), &m_rCookie.name);
	}

	virtual void SetData(const char* data)
	{
		cef_string_copy(data, strlen(data), &m_rCookie.value);
	}

	virtual void SetPath(const char* path)
	{
		cef_string_copy(path, strlen(path), &m_rCookie.path);
	}

	CefCookie m_rCookie;
};

class CookieTask : public CefRefCountWrapper<CefTask>
{
public:
	CookieTask(const char* url, CefCookie &cookie)
	{
		m_szCookie = cookie;

		if (url)
			m_szUrl = url;

		m_bDel = false;
	}

	CookieTask(const char* url, const char* name)
	{
		if (url)
			m_szUrl = url;

		if (name)
			m_szName = name;
	
		m_bDel = true;
	}

	virtual void Execute(CefThreadId threadId)
	{
		if (m_szName.size())
			CefDeleteCookies(m_szUrl.c_str(), m_szName.c_str());
		else
			CefSetCookie(m_szUrl.c_str(), m_szCookie);
	}

	bool m_bDel;

	std::string m_szUrl;
	std::string m_szName;

	CefCookie m_szCookie;
};

extern "C"
{
	DLLINTERFACE void CEF_DeleteCookie(const char* url, const char* name)
	{
		CefPostTask(TID_IO, new CookieTask(url, name));
	}

	DLLINTERFACE ChromiumDLL::CookieI* CEF_CreateCookie()
	{
		return new Cookie();
	}

	DLLINTERFACE void CEF_SetCookie(const char* url, ChromiumDLL::CookieI* cookie)
	{
		Cookie* c = (Cookie*)cookie;

		if (!c)
			return;

		CefPostTask(TID_IO, new CookieTask(url, c->m_rCookie));
	}
}
