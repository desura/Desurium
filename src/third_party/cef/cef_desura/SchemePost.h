///////////// Copyright © 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : desura_libcef_dll_wrapper
//   File        : SchemePost.h
//   Description :
//      [TODO: Write the purpose of SchemePost.h.]
//
//   Created On: 6/17/2010 5:04:50 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#ifndef DESURA_SCHEMEPOST_H
#define DESURA_SCHEMEPOST_H
#ifdef _WIN32
#pragma once
#endif

#include "ChromiumBrowserI.h"
#include "include/cef.h"

class PostElement : public ChromiumDLL::PostElementI
{
public:
	PostElement();
	PostElement(CefRefPtr<CefPostDataElement> element);

	//! Deletes the object. Should never be called by user code!
	//!
	virtual void destroy()
	{
		delete this;
	}

	virtual bool isFile();
	virtual bool isBytes();

	virtual void setToEmpty();
	virtual void setToFile(const char* fileName);
	virtual void setToBytes(size_t size, const void* bytes);

	virtual void getFile(char *buff, size_t buffsize);

	virtual size_t getBytesCount();
	virtual size_t getBytes(size_t size, void* bytes);

	CefRefPtr<CefPostDataElement> getHandle()
	{
		return m_rPostElement;
	}

private:
	CefRefPtr<CefPostDataElement> m_rPostElement;
};


class PostData : public ChromiumDLL::PostDataI
{
public:
	PostData();
	PostData(CefRefPtr<CefPostData> data);


	//! Deletes the object. Should never be called by user code!
	//!
	virtual void destroy()
	{
		delete this;
	}

	virtual size_t getElementCount();
	virtual ChromiumDLL::PostElementI* getElement(size_t index);

	virtual bool removeElement(ChromiumDLL::PostElementI* element);
	virtual bool addElement(ChromiumDLL::PostElementI* element);

	virtual void removeElements();

	CefRefPtr<CefPostData> getHandle()
	{
		return m_rPostData;
	}

private:
	CefRefPtr<CefPostData> m_rPostData;
};





#endif //DESURA_SCHEMEPOST_H
