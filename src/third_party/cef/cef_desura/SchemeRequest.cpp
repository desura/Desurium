///////////// Copyright © 2010 Desura Pty Ltd. All rights reserved.  /////////////
//
//   Project     : desura_libcef_dll_wrapper
//   File        : SchemeRequest.cpp
//   Description :
//      [TODO: Write the purpose of SchemeRequest.cpp.]
//
//   Created On: 6/17/2010 4:48:42 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#include "SchemeRequest.h"
#include "SchemePost.h"

#define _CRT_SECURE_NO_WARNINGS

int mystrncpy_s(char* dest, size_t destSize, const char* src, size_t srcSize);

SchemeRequest::SchemeRequest()
{

}

SchemeRequest::SchemeRequest(CefRefPtr<CefRequest> request)
{
	m_rRequest = request;
}


void SchemeRequest::getURL(char *buff, size_t buffsize)
{
	std::string url = m_rRequest->GetURL();

	if (buff)
		mystrncpy_s(buff, buffsize, url.c_str(), url.size());
}

void SchemeRequest::setURL(const char* url)
{
	m_rRequest->SetURL(url);
}


void SchemeRequest::getMethod(char *buff, size_t buffsize)
{
	std::string method = m_rRequest->GetMethod();

	if (buff)
		mystrncpy_s(buff, buffsize, method.c_str(), method.size());
}

void SchemeRequest::setMethod(const char* method)
{
	m_rRequest->SetMethod(method);
}


ChromiumDLL::PostDataI* SchemeRequest::getPostData()
{
	return new PostData(m_rRequest->GetPostData());
}

void SchemeRequest::setPostData(ChromiumDLL::PostDataI* postData)
{
	if (postData)
	{
		PostData* pd = (PostData*)postData;

		if (pd)
			m_rRequest->SetPostData(pd->getHandle());

		postData->destroy();
	}
}

size_t SchemeRequest::getHeaderCount()
{
	CefRequest::HeaderMap map;
	m_rRequest->GetHeaderMap(map);

	return map.size();
}


void SchemeRequest::getHeaderItem(size_t index, char *key, size_t keysize, char* data, size_t datasize)
{
	CefRequest::HeaderMap map;
	m_rRequest->GetHeaderMap(map);

	if (map.size() >= index)
		return;


	CefRequest::HeaderMap::iterator it = map.begin();

	for (size_t x=0; x<index; x++)
		;

	if (it == map.end())
		return;

	if (key)
		mystrncpy_s(key, keysize, (*it).first.c_str(), (*it).first.size());

	if (data)
		mystrncpy_s(data, datasize, (*it).second.c_str(), (*it).second.size());
}

void SchemeRequest::setHeaderItem(const char* key, const char* data)
{
	CefRequest::HeaderMap map;
	m_rRequest->GetHeaderMap(map);

	map[key] = data;

	m_rRequest->SetHeaderMap(map);
}

void SchemeRequest::set(const char* url, const char* method, ChromiumDLL::PostDataI* postData)
{
	setURL(url);
	setMethod(method);
	setPostData(postData);
}