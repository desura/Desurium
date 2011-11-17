///////////// Copyright © 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : desura_libcef_dll_wrapper
//   File        : SchemeExtender.h
//   Description :
//      [TODO: Write the purpose of SchemeExtender.h.]
//
//   Created On: 6/17/2010 4:32:18 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef DESURA_SCHEMEEXTENDER_H
#define DESURA_SCHEMEEXTENDER_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
#include "include/cef.h"


class SchemeExtender : public CefRefCountWrapper<CefSchemeHandler>, public ChromiumDLL::SchemeCallbackI
{
public:
	static bool Register(ChromiumDLL::SchemeExtenderI* se);

	SchemeExtender(ChromiumDLL::SchemeExtenderI* se);
	~SchemeExtender();


	virtual bool ProcessRequest(CefRefPtr<CefRequest> request, CefString& redirectUrl, CefRefPtr<CefSchemeHandlerCallback> callback);
	virtual void Cancel();

	virtual void GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length);
	virtual bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefSchemeHandlerCallback> callback);

	virtual void responseReady();
	virtual void dataReady();
	virtual void cancel();

private:
	ChromiumDLL::SchemeExtenderI* m_pSchemeExtender;
	CefRefPtr<CefSchemeHandlerCallback> m_Callback;
};


#endif //DESURA_SCHEMEEXTENDER_H
