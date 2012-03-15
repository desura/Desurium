///////////// Copyright © 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : desura_libcef_dll_wrapper
//   File        : SchemeRequest.h
//   Description :
//      [TODO: Write the purpose of SchemeRequest.h.]
//
//   Created On: 6/17/2010 4:46:02 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef DESURA_SCHEMEREQUEST_H
#define DESURA_SCHEMEREQUEST_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
#include "include/cef.h"

class SchemeRequest : public ChromiumDLL::SchemeRequestI
{
public:
	SchemeRequest();
	SchemeRequest(CefRefPtr<CefRequest> request);

	virtual void destroy()
	{
		delete this;
	}

	virtual void getURL(char *buff, size_t buffsize);
	virtual void setURL(const char* url);

	virtual void getMethod(char *buff, size_t buffsize);
	virtual void setMethod(const char* method);

	virtual ChromiumDLL::PostDataI* getPostData();
	virtual void setPostData(ChromiumDLL::PostDataI* postData);

	virtual size_t getHeaderCount();

	virtual void getHeaderItem(size_t index, char *key, size_t keysize, char* data, size_t datasize);
	virtual void setHeaderItem(const char* key, const char* data);

	virtual void set(const char* url, const char* method, ChromiumDLL::PostDataI* postData);

	CefRefPtr<CefRequest> getHandle()
	{
		return m_rRequest;
	}

private:
	CefRefPtr<CefRequest> m_rRequest;
};


#endif //DESURA_SCHEMEREQUEST_H
